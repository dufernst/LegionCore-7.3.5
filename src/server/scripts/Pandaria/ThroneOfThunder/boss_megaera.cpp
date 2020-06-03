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
    //Flame Head
    SPELL_IGNITE_FLESH        = 137729, 
    SPELL_CINDERS_DOT         = 139822,
    SPELL_CINDERS_AURA_DMG    = 139835,
    SPELL_FL_MEGAERA_RAGE     = 139758,
    //Frozen Head
    SPELL_ARCTIC_FREEZE       = 139841,
    SPELL_FR_MEGAERA_RAGE     = 139816,
    SPELL_TORRENT_OF_ICE_CH   = 139866, //channel - beam visual
    SPELL_TORRENT_OF_ICE_AURA = 139890,
    SPELL_ICY_GROUND_AT       = 139875,
    SPELL_ICY_GROUND_DMG      = 139909,
    //Venomous Head
    SPELL_ROT_ARMOR           = 139838, 
    SPELL_V_MEGAERA_RAGE      = 139818,
    SPELL_ACID_RAIN_S_VISUAL  = 139847,
    SPELL_ACID_RAIN_EXPLOSE   = 139850,
    SPELL_ACID_RAIN_TR_M      = 139848,
    //Special
    SPELL_FLAME_EBOM          = 139586,
    SPELL_FROZEN_EBOM         = 139587,
    SPELL_VENOMOUSE_EBOM      = 139588,
    //Megaera Rampage Spells
    SPELL_MEGAERA_RAMPAGE     = 139458,
    SPELL_FLAME_RAMPAGE_DMG   = 139548,
    SPELL_FROZEN_RAMPAGE_DMG  = 139549,
    SPELL_VENOMOUSE_RAMPAGE_D = 139551,
    SPELL_FL_VISUAL_RAMPAGE   = 139433,
    SPELL_V_VISUAL_RAMPAGE    = 139504,
    SPELL_FR_VISUAL_RAMPAGE   = 139440,
};

enum sEvents
{
    //All
    EVENT_BREATH              = 1,
    //Flame
    EVENT_CINDERS             = 2,
    //Venomous
    EVENT_ACID_RAIN           = 3,
    //Frozen
    EVENT_TORRENT_OF_ICE      = 4,
    //Special
    EVENT_SPAWN_NEW_HEADS     = 5,
    EVENT_DESPAWN             = 6,
    EVENT_ACTIVE_PURSUIT      = 7,
    EVENT_MEGAERA_RAMPAGE     = 8,
    EVENT_RESET_EVENT         = 9,
    EVENT_RESTART_EVENTS      = 10,
};

uint32 const megaeraheads[6] =
{
    NPC_FLAMING_HEAD_MELEE,
    NPC_FROZEN_HEAD_MELEE,
    NPC_VENOMOUS_HEAD_MELEE,
    NPC_FLAMING_HEAD_RANGE,
    NPC_VENOMOUS_HEAD_RANGE,
    NPC_FROZEN_HEAD_RANGE,
};

class MegaeraTankFilter
{
public:
    bool operator()(WorldObject* unit)
    {
        if (Player* target = unit->ToPlayer())
            if (!target->isInTankSpec())
                return false;
        return true;
    }
};

//68065
class npc_megaera : public CreatureScript
{
public:
    npc_megaera() : CreatureScript("npc_megaera") { }

    struct npc_megaeraAI : public ScriptedAI
    {
        npc_megaeraAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            instance = me->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NON_ATTACKABLE);
            me->SetDisplayId(48113);
        }
        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            events.Reset();
            me->setPowerType(POWER_ENERGY);
            me->SetPower(POWER_ENERGY, 0);

            if (instance->GetBossState(DATA_MEGAERA != NOT_STARTED))
                instance->SetBossState(DATA_MEGAERA, NOT_STARTED);

            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_MEGAERA_IN_PROGRESS:
                DoZoneInCombat(me, 150.0f);
                break;
            case ACTION_MEGAERA_RESET:
                if (me->isInCombat())
                    EnterEvadeMode();
                break;
            case ACTION_MEGAERA_RAMPAGE:
                events.RescheduleEvent(EVENT_MEGAERA_RAMPAGE, 2000);
                break;
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void EnterCombat(Unit* /*victim*/)
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_MEGAERA_RAMPAGE:
                {
                    std::list<Creature*>megaeraheadlist;
                    megaeraheadlist.clear();
                    for (uint8 n = 0; n < 6; n++)
                        GetCreatureListWithEntryInGrid(megaeraheadlist, me, megaeraheads[n], 200.0f);

                    if (!megaeraheadlist.empty())
                        for (std::list<Creature*>::const_iterator itr = megaeraheadlist.begin(); itr != megaeraheadlist.end(); itr++)
                            (*itr)->AI()->DoAction(ACTION_MEGAERA_RAMPAGE);

                    events.RescheduleEvent(EVENT_RESTART_EVENTS, 22000);
                }
                break;
                case EVENT_RESTART_EVENTS:
                {
                    std::list<Creature*>megaeraheadlist;
                    megaeraheadlist.clear();
                    for (uint8 n = 0; n < 6; n++)
                        GetCreatureListWithEntryInGrid(megaeraheadlist, me, megaeraheads[n], 200.0f);

                    if (!megaeraheadlist.empty())
                        for (std::list<Creature*>::const_iterator itr = megaeraheadlist.begin(); itr != megaeraheadlist.end(); itr++)
                            (*itr)->AI()->DoAction(ACTION_RESTART_EVENTS);
                }
                break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_megaeraAI(pCreature);
    }
};

struct megaera_headAI : public ScriptedAI
{
    megaera_headAI(Creature* creature) : ScriptedAI(creature)
    {
        instance = creature->GetInstanceScript();
    }
    InstanceScript* instance;

    void MegaeraHeadEnterCombat()
    {
        for (uint8 i = 0; i < 6; i++)
            if (Creature* megaerahead = me->GetCreature(*me, instance->GetGuidData(megaeraheads[i])))
                if (me->GetEntry() != megaerahead->GetEntry())
                    if (megaerahead->isAlive() && !megaerahead->isInCombat())
                        DoZoneInCombat(megaerahead, 150.0f);

        if (instance->GetBossState(DATA_MEGAERA != IN_PROGRESS))
            instance->SetBossState(DATA_MEGAERA, IN_PROGRESS);
    }
};

class npc_megaera_head : public CreatureScript
{
public:
    npc_megaera_head() : CreatureScript("npc_megaera_head") {}

    struct npc_megaera_headAI : public megaera_headAI
    {
        npc_megaera_headAI(Creature* creature) : megaera_headAI(creature), summon(me)
        {
            instance = creature->GetInstanceScript();
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetStandState(UNIT_STAND_STATE_SUBMERGED);
            timermod = 0;
            //megaerarampage = false;
        }
        InstanceScript* instance;
        SummonList summon;
        EventMap events;
        uint32 checkvictim;
        uint32 spawntimer;
        uint32 nextheadentry;
        uint32 timermod;
        bool done;
        //bool megaerarampage;

        void Reset()
        {
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            summon.DespawnAll();
            events.Reset();
            me->setFaction(16);
            me->SetReactState(REACT_DEFENSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            spawntimer = 2000;
            checkvictim = 0;
            nextheadentry = 0;
            done = false;
        }

        void EnterCombat(Unit* who)
        {
            MegaeraHeadEnterCombat();
            switch (me->GetEntry())
            {
            case NPC_FLAMING_HEAD_MELEE:
            case NPC_VENOMOUS_HEAD_MELEE:
            case NPC_FROZEN_HEAD_MELEE:
                events.RescheduleEvent(EVENT_BREATH, 6000);
                checkvictim = 4000;
                break;
            case NPC_FLAMING_HEAD_RANGE:
                events.RescheduleEvent(EVENT_CINDERS, 25000 + timermod);
                break;
            case NPC_VENOMOUS_HEAD_RANGE:
                events.RescheduleEvent(EVENT_ACID_RAIN, 26000 + timermod);
                break;
            case NPC_FROZEN_HEAD_RANGE:
                events.RescheduleEvent(EVENT_TORRENT_OF_ICE, 27000 + timermod);
                break;
            }
        }

        void UpdateElementalBloodofMegaera()
        {
            uint8 stack = instance->GetData(me->GetEntry());
            if (stack)
            {
                uint32 spellentry = 0;
                switch (me->GetEntry())
                {
                case NPC_FLAMING_HEAD_MELEE:
                    spellentry = SPELL_FLAME_EBOM;
                    break;
                case NPC_FROZEN_HEAD_MELEE:
                    spellentry = SPELL_FROZEN_EBOM;
                    break;
                case NPC_VENOMOUS_HEAD_MELEE:
                    spellentry = SPELL_VENOMOUSE_EBOM;
                    break;
                default:
                    break;
                }
                me->CastCustomSpell(spellentry, SPELLVALUE_AURA_STACK, stack, me);
            }
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
            case DATA_UPDATE_MOD_TIMER:
                timermod = data;
                DoZoneInCombat(me, 150.0f);
                break;
            case DATA_SPAWN_NEW_HEAD:
                //megaerarampage = true;
                DoZoneInCombat(me, 150.0f);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                break;
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            /*//Megaera head die only after rampage remove...
            if (damage >= me->GetHealth() && me->HasAura(SPELL_MEGAERA_RAMPAGE))
            {
                damage = 0;
                return;
            }*/

            if (damage >= me->GetHealth() && !done)
            {
                damage = 0;
                done = true;
                me->RemoveAllAuras();
                me->StopAttack();
                events.Reset();
                me->InterruptNonMeleeSpells(true);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                me->SetStandState(UNIT_STAND_STATE_DEAD);
                instance->SetData(DATA_SEND_LAST_DIED_HEAD, me->GetEntry());
                if (instance->GetData(DATA_CHECK_PROGRESS_MEGAERA))
                    GetAndActiveNextHead();
            }

            if (damage >= me->GetHealth())
                damage = 0;
        }

        void GetAndActiveNextHead()
        {
            if (Creature* nexthead = me->GetCreature(*me, instance->GetGuidData(DATA_GET_NEXT_HEAD)))
            {
                nexthead->AI()->DoAction(ACTION_UNSUMMON);
                nextheadentry = GetNextMeleeHeadEntry(nexthead->GetEntry());
                if (Creature* megaera = me->GetCreature(*me, instance->GetGuidData(NPC_MEGAERA)))
                {
                    uint32 modhp = megaera->CountPctFromMaxHealth(14.3f);
                    megaera->SetHealth(megaera->GetHealth() - modhp);
                    events.RescheduleEvent(EVENT_SPAWN_NEW_HEADS, 3000);
                }
            }
        }

        uint32 GetNextMeleeHeadEntry(uint32 megaeraheadentry)
        {
            switch (megaeraheadentry)
            {
            case NPC_FLAMING_HEAD_RANGE:
                return NPC_FLAMING_HEAD_MELEE;
            case NPC_FROZEN_HEAD_RANGE:
                return NPC_FROZEN_HEAD_MELEE;
            case NPC_VENOMOUS_HEAD_RANGE:
                return NPC_VENOMOUS_HEAD_MELEE;
            default:
                return 0;
            }
        }

        void EnterEvadeMode()
        {
            ScriptedAI::EnterEvadeMode();

            if (instance->GetBossState(DATA_MEGAERA) != FAIL)
                instance->SetBossState(DATA_MEGAERA, FAIL);
        }

        void JustSummoned(Creature* summons)
        {
            summon.Summon(summons);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_UNSUMMON:
                events.Reset();
                checkvictim = 0;
                me->StopAttack();
                me->InterruptNonMeleeSpells(true);
                me->RemoveAllAuras();
                me->SetStandState(UNIT_STAND_STATE_SUBMERGED);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                events.RescheduleEvent(EVENT_DESPAWN, 2000);
                break;
            case ACTION_MEGAERA_DONE:
                events.Reset();
                me->InterruptNonMeleeSpells(true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->RemoveAllAuras();
                me->SetStandState(UNIT_STAND_STATE_DEAD);
                events.RescheduleEvent(EVENT_DESPAWN, 3000);
                break;
            case ACTION_MEGAERA_RAMPAGE:
                checkvictim = 0;
                events.Reset();
                me->InterruptNonMeleeSpells(true);
                me->StopAttack();
                DoCast(me, SPELL_MEGAERA_RAMPAGE, true);
                events.RescheduleEvent(EVENT_RESET_EVENT, 21000);
                break;
            case ACTION_RESTART_EVENTS:
                me->SetReactState(REACT_AGGRESSIVE);
                switch (me->GetEntry())
                {
                case NPC_FLAMING_HEAD_MELEE:
                case NPC_VENOMOUS_HEAD_MELEE:
                case NPC_FROZEN_HEAD_MELEE:
                    events.RescheduleEvent(EVENT_BREATH, 10000);
                    checkvictim = 4000;
                    break;
                case NPC_FLAMING_HEAD_RANGE:
                    events.RescheduleEvent(EVENT_CINDERS, 16000 + timermod);
                    break;
                case NPC_VENOMOUS_HEAD_RANGE:
                    events.RescheduleEvent(EVENT_ACID_RAIN, 17000 + timermod);
                    break;
                case NPC_FROZEN_HEAD_RANGE:
                    events.RescheduleEvent(EVENT_TORRENT_OF_ICE, 18000 + timermod);
                    break;
                }
                break;
            case ACTION_PREPARE_TO_UNSUMMON:
                events.Reset();
                checkvictim = 0;
                me->StopAttack();
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                me->InterruptNonMeleeSpells(true);
                me->RemoveAllAuras();
                me->SetDisplayId(11686);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                break;
            }
        }

        uint32 GetRageSpellEntry(uint32 megaeraheadentry)
        {
            switch (megaeraheadentry)
            {
            case NPC_FLAMING_HEAD_MELEE:
                return SPELL_FL_MEGAERA_RAGE;
            case NPC_VENOMOUS_HEAD_MELEE:
                return SPELL_V_MEGAERA_RAGE;
            case NPC_FROZEN_HEAD_MELEE:
                return SPELL_FR_MEGAERA_RAGE;
            default:
                return 0;
            }
        }

        bool IsPlayerRangeDDOrHeal(Player* player)
        {
            switch (player->getClass())
            {
            case CLASS_PRIEST:
            case CLASS_WARLOCK:
            case CLASS_MAGE:
            case CLASS_HUNTER:
                return true;
            case CLASS_PALADIN:
                if (player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == SPEC_PALADIN_HOLY)
                    return true;
            case CLASS_MONK:
                if (player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == SPEC_MONK_MISTWEAVER)
                    return true;
            case CLASS_SHAMAN:
                if (player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == SPEC_SHAMAN_ELEMENTAL || player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == SPEC_SHAMAN_RESTORATION)
                    return true;
            case CLASS_DRUID:
                if (player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == SPEC_DRUID_RESTORATION || player->GetUInt32Value(PLAYER_FIELD_CURRENT_SPEC_ID) == SPEC_DRUID_BALANCE)
                    return true;
            default:
                return false;
            }
        }

        void UpdateAI(uint32 diff)
        {
            if (spawntimer)
            {
                if (spawntimer <= diff)
                {
                    spawntimer = 0;
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    switch (me->GetEntry())
                    {
                    case NPC_FLAMING_HEAD_MELEE:
                    case NPC_VENOMOUS_HEAD_MELEE:
                    case NPC_FROZEN_HEAD_MELEE:
                        UpdateElementalBloodofMegaera();
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        /*if (megaerarampage)
                            if (Creature* megaera = me->GetCreature(*me, instance->GetGuidData(NPC_MEGAERA)))
                                megaera->AI()->DoAction(ACTION_MEGAERA_RAMPAGE);*/
                        break;
                    default:
                        break;
                    }
                }
                else
                    spawntimer -= diff;
            }

            if (!UpdateVictim())
                return;

            if (checkvictim)
            {
                if (checkvictim <= diff)
                {
                    if (me->getVictim() && !me->IsWithinMeleeRange(me->getVictim()))
                    {
                        uint32 spellentry = GetRageSpellEntry(me->GetEntry());
                        DoCast(me->getVictim(), spellentry);
                    }
                    checkvictim = 4000;
                }
                else
                    checkvictim -= diff;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_BREATH:
                {
                    switch (me->GetEntry())
                    {
                    case NPC_FLAMING_HEAD_MELEE:
                        DoCast(me, SPELL_IGNITE_FLESH);
                        break;
                    case NPC_FROZEN_HEAD_MELEE:
                        DoCast(me, SPELL_ARCTIC_FREEZE);
                        break;
                    case NPC_VENOMOUS_HEAD_MELEE:
                        DoCast(me, SPELL_ROT_ARMOR);
                        break;
                    }
                    events.RescheduleEvent(EVENT_BREATH, 15000);
                    break;
                }
                case EVENT_CINDERS:
                {
                    std::list<Player*>pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 150.0f);
                    pllist.remove_if(MegaeraTankFilter());
                    bool havetarget = false;
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                        {
                            if (!(*itr)->HasAura(SPELL_CINDERS_DOT) && IsPlayerRangeDDOrHeal(*itr))
                            {
                                DoCast(*itr, SPELL_CINDERS_DOT);
                                havetarget = true;
                                break;
                            }
                        }
                        if (!havetarget)
                        {
                            for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                            {
                                if (!(*itr)->HasAura(SPELL_CINDERS_DOT))
                                {
                                    DoCast(*itr, SPELL_CINDERS_DOT);
                                    break;
                                }
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_CINDERS, 25000 + timermod);
                }
                break;
                case EVENT_ACID_RAIN:
                {
                    std::list<Player*>pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 150.0f);
                    pllist.remove_if(MegaeraTankFilter());
                    bool havetarget = false;
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                        {
                            if (!(*itr)->HasAura(SPELL_TORRENT_OF_ICE_T) && IsPlayerRangeDDOrHeal(*itr))
                            {
                                Position pos;
                                (*itr)->GetPosition(&pos);
                                if (Creature* ar = me->SummonCreature(NPC_ACID_RAIN, pos, TEMPSUMMON_TIMED_DESPAWN, 15000))
                                    DoCast(ar, SPELL_ACID_RAIN_TR_M);
                                havetarget = true;
                                break;
                            }
                        }
                        if (!havetarget)
                        {
                            std::list<Player*>::iterator itr = pllist.begin();
                            std::advance(itr, urand(0, pllist.size() - 1));
                            Position pos;
                            (*itr)->GetPosition(&pos);
                            if (Creature* ar = me->SummonCreature(NPC_ACID_RAIN, pos, TEMPSUMMON_TIMED_DESPAWN, 15000))
                                DoCast(ar, SPELL_ACID_RAIN_TR_M);
                        }
                    }
                    events.RescheduleEvent(EVENT_ACID_RAIN, 26000 + timermod);
                }
                break;
                case EVENT_TORRENT_OF_ICE:
                {
                    std::list<Player*>pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 150.0f);
                    pllist.remove_if(MegaeraTankFilter());
                    bool havetarget = false;
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                        {
                            if (!(*itr)->HasAura(SPELL_TORRENT_OF_ICE_T) && IsPlayerRangeDDOrHeal(*itr))
                            {
                                if (Creature* torrent = me->SummonCreature(NPC_TORRENT_OF_ICE, (*itr)->GetPositionX() + 10.0f, (*itr)->GetPositionY(), (*itr)->GetPositionZ()))
                                {
                                    torrent->AI()->SetGUID((*itr)->GetGUID(), 1);
                                    DoCast(torrent, SPELL_TORRENT_OF_ICE_CH);
                                    havetarget = true;
                                    break;
                                }
                            }
                        }
                        if (!havetarget)
                        {
                            for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                            {
                                if (!(*itr)->HasAura(SPELL_TORRENT_OF_ICE_T))
                                {
                                    if (Creature* torrent = me->SummonCreature(NPC_TORRENT_OF_ICE, (*itr)->GetPositionX() + 10.0f, (*itr)->GetPositionY(), (*itr)->GetPositionZ()))
                                    {
                                        torrent->AI()->SetGUID((*itr)->GetGUID(), 1);
                                        DoCast(torrent, SPELL_TORRENT_OF_ICE_CH);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_TORRENT_OF_ICE, 27000 + timermod);
                }
                break;
                case EVENT_DESPAWN:
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_SPAWN_NEW_HEADS:
                    if (Creature* megaera = me->GetCreature(*me, instance->GetGuidData(NPC_MEGAERA)))
                    {
                        uint8 posmod = 6;
                        if (uint32(me->GetPositionX()) == 6438)      //left
                            posmod = 4;
                        else if (uint32(me->GetPositionX()) == 6419) //right
                            posmod = 5;
                        else if (uint32(me->GetPositionX()) == 6437) //left++
                            posmod = 0;
                        else if (uint32(me->GetPositionX()) == 6394) //right++
                            posmod = 1;

                        if (Creature* mh = megaera->SummonCreature(nextheadentry, megaeraspawnpos[posmod]))
                            mh->AI()->SetData(DATA_SPAWN_NEW_HEAD, 0);

                        uint32 newheadsentry = 0;
                        switch (me->GetEntry())
                        {
                        case NPC_FLAMING_HEAD_MELEE:
                            newheadsentry = NPC_FLAMING_HEAD_RANGE;
                            break;
                        case NPC_VENOMOUS_HEAD_MELEE:
                            newheadsentry = NPC_VENOMOUS_HEAD_RANGE;
                            break;
                        case NPC_FROZEN_HEAD_MELEE:
                            newheadsentry = NPC_FROZEN_HEAD_RANGE;
                            break;
                        default:
                            break;
                        }

                        if (!instance->GetData(DATA_GET_COUNT_RANGE_HEADS))
                        {
                            for (uint8 n = 2; n < 4; n++)
                                if (Creature* mh2 = megaera->SummonCreature(newheadsentry, megaeraspawnpos[n]))
                                    mh2->AI()->SetData(DATA_UPDATE_MOD_TIMER, (n - 2)* 2000);
                        }
                        else
                        {
                            std::list<Creature*>rangemhlist;
                            rangemhlist.clear();
                            GetCreatureListWithEntryInGrid(rangemhlist, me, NPC_FLAMING_HEAD_RANGE, 150.0f);
                            GetCreatureListWithEntryInGrid(rangemhlist, me, NPC_FROZEN_HEAD_RANGE, 150.0f);
                            GetCreatureListWithEntryInGrid(rangemhlist, me, NPC_VENOMOUS_HEAD_RANGE, 150.0f);
                            if (!rangemhlist.empty())
                            {
                                bool blockpos = false;
                                uint8 count = 0;
                                for (uint8 n = 0; n < 8; n++, blockpos = false)
                                {
                                    for (std::list<Creature*>::const_iterator itr = rangemhlist.begin(); itr != rangemhlist.end(); itr++)
                                    {
                                        if (uint32((*itr)->GetPositionX()) == uint32(megaerarangespawnpos[n].GetPositionX()))
                                            blockpos = true;
                                    }

                                    if (!blockpos && count < 2)
                                    {
                                        count++;
                                        if (Creature* mh3 = megaera->SummonCreature(newheadsentry, megaerarangespawnpos[n]))
                                            mh3->AI()->SetData(DATA_UPDATE_MOD_TIMER, count * 2000);
                                    }

                                    if (count == 2)
                                        break;
                                }
                            }
                        }
                    }
                    me->DespawnOrUnsummon();
                    break;
                }
            }
            if (!me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_megaera_headAI(creature);
    }
};

//70432
class npc_cinders : public CreatureScript
{
public:
    npc_cinders() : CreatureScript("npc_cinders") { }

    struct npc_cindersAI : public ScriptedAI
    {
        npc_cindersAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = (InstanceScript*)pCreature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }
        InstanceScript* m_pInstance;
        uint32 despawn;

        void Reset()
        {
            me->SetFloatValue(OBJECT_FIELD_SCALE, 0.4f);
            me->AddAura(SPELL_CINDERS_AURA_DMG, me);
            despawn = 60000;
        }

        void EnterCombat(Unit* /*victim*/){}

        void EnterEvadeMode() {}

        void UpdateAI(uint32 diff)
        {
            if (despawn <= diff)
                me->DespawnOrUnsummon();
            else
                despawn -= diff;
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_cindersAI(pCreature);
    }
};

//70435
class npc_acid_rain : public CreatureScript
{
public:
    npc_acid_rain() : CreatureScript("npc_acid_rain") { }

    struct npc_acid_rainAI : public ScriptedAI
    {
        npc_acid_rainAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = (InstanceScript*)pCreature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE);
        }
        InstanceScript* m_pInstance;
        uint32 despawn;
        bool done;

        void Reset()
        {
            done = false;
            despawn = 0;
            DoCast(me, SPELL_ACID_RAIN_S_VISUAL, true);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void EnterCombat(Unit* /*victim*/){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_acid_rainAI(pCreature);
    }
};

//70439
class npc_torrent_of_ice : public CreatureScript
{
public:
    npc_torrent_of_ice() : CreatureScript("npc_torrent_of_ice") { }

    struct npc_torrent_of_iceAI : public ScriptedAI
    {
        npc_torrent_of_iceAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = (InstanceScript*)pCreature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }
        InstanceScript* m_pInstance;
        ObjectGuid targetGuid;
        EventMap events;

        void Reset()
        {
            events.Reset();
            targetGuid.Clear();
        }

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            targetGuid = guid;
            if (Player* pl = me->GetPlayer(*me, guid))
            {
                if (pl->isAlive())
                {
                    DoCast(pl, SPELL_TORRENT_OF_ICE_T, true);
                    events.RescheduleEvent(EVENT_ACTIVE_PURSUIT, 1000);
                    return;
                }
            }
            me->DespawnOrUnsummon();
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void EnterCombat(Unit* /*victim*/){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_ACTIVE_PURSUIT:
                    if (Player* player = me->GetPlayer(*me, targetGuid))
                    {
                        if (player->isAlive())
                        {
                            DoCast(me, SPELL_TORRENT_OF_ICE_AURA, true);
                            me->AddThreat(player, 50000000.0f);
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->Attack(player, true);
                            me->GetMotionMaster()->MoveChase(player);
                        }
                    }
                    events.RescheduleEvent(EVENT_DESPAWN, 8200);
                    break;
                case EVENT_DESPAWN:
                    if (Player* player = me->GetPlayer(*me, targetGuid))
                        if (player->isAlive())
                            player->RemoveAurasDueToSpell(SPELL_TORRENT_OF_ICE_T);
                    me->DespawnOrUnsummon();
                    break;   
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_torrent_of_iceAI(pCreature);
    }
};

//70446
class npc_icy_ground : public CreatureScript
{
public:
    npc_icy_ground() : CreatureScript("npc_icy_ground") { }

    struct npc_icy_groundAI : public ScriptedAI
    {
        npc_icy_groundAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = (InstanceScript*)pCreature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE);
        }
        InstanceScript* m_pInstance;

        void Reset()
        {
            DoCast(me, SPELL_ICY_GROUND_AT, true);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void EnterCombat(Unit* /*victim*/){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_icy_groundAI(pCreature);
    }
};

//139822
class spell_cinders : public SpellScriptLoader
{
public:
    spell_cinders() : SpellScriptLoader("spell_cinders") { }

    class spell_cinders_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_cinders_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster() && GetTarget())
                GetCaster()->SummonCreature(NPC_CINDERS, GetTarget()->GetPositionX(), GetTarget()->GetPositionY(), GetTarget()->GetPositionZ());
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_cinders_AuraScript::HandleEffectRemove, EFFECT_1, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_cinders_AuraScript();
    }
};

//139850
class spell_acid_rain : public SpellScriptLoader
{
public:
    spell_acid_rain() : SpellScriptLoader("spell_acid_rain") { }

    class spell_acid_rain_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_acid_rain_SpellScript);

        void DealDamage()
        {
            if (GetCaster() && GetHitUnit())
            {
                float distance = GetCaster()->GetExactDist2d(GetHitUnit());
                if (distance)
                {
                    if (distance <= 5)
                        SetHitDamage(GetHitDamage() * (1 - (distance / 200)));
                    else if (distance > 5 && distance <= 10)
                        SetHitDamage((GetHitDamage()/1.5) * (1 - (distance / 200)));
                    else if (distance > 10)
                        SetHitDamage((GetHitDamage()/2) * (1 - (distance / 200)));
                }
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_acid_rain_SpellScript::DealDamage);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_acid_rain_SpellScript();
    }
};

class TorrentOfIceFilterTarget
{
public:
    bool operator()(WorldObject* unit) const
    {
        if (unit->ToCreature() && unit->GetEntry() == NPC_CINDERS)
            return false;
        return true;
    }
};

//139889
class spell_torrent_of_ice : public SpellScriptLoader
{
public:
    spell_torrent_of_ice() : SpellScriptLoader("spell_torrent_of_ice") { }

    class spell_torrent_of_ice_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_torrent_of_ice_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            if (GetHitUnit() && GetHitUnit()->ToCreature()) //for safe
                GetHitUnit()->ToCreature()->DespawnOrUnsummon();
        }

        void FilterTarget(std::list<WorldObject*>&targets)
        {
            targets.remove_if(TorrentOfIceFilterTarget());
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_torrent_of_ice_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_torrent_of_ice_SpellScript::FilterTarget, EFFECT_1, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_torrent_of_ice_SpellScript();
    }
};

//139458
class spell_megaera_rampage : public SpellScriptLoader
{
public:
    spell_megaera_rampage() : SpellScriptLoader("spell_megaera_rampage") { }

    class spell_megaera_rampage_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_megaera_rampage_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                uint32 spellentry = 0;
                uint32 visualspell = 0;
                uint32 searchentry = 0;
                uint32 searchentry2 = 0;

                switch (GetCaster()->ToCreature()->GetEntry())
                {
                case NPC_FLAMING_HEAD_MELEE:
                case NPC_FLAMING_HEAD_RANGE:
                    spellentry = SPELL_FLAME_RAMPAGE_DMG;
                    visualspell = SPELL_FL_VISUAL_RAMPAGE;
                    searchentry = NPC_FLAMING_HEAD_MELEE;
                    searchentry2 = NPC_FLAMING_HEAD_RANGE;
                    break;
                case NPC_VENOMOUS_HEAD_MELEE:
                case NPC_VENOMOUS_HEAD_RANGE:
                    spellentry = SPELL_VENOMOUSE_RAMPAGE_D;
                    visualspell = SPELL_V_VISUAL_RAMPAGE;
                    searchentry = NPC_VENOMOUS_HEAD_MELEE;
                    searchentry2 = NPC_VENOMOUS_HEAD_RANGE;
                    break;
                case NPC_FROZEN_HEAD_MELEE:
                case NPC_FROZEN_HEAD_RANGE:
                    spellentry = SPELL_FROZEN_RAMPAGE_DMG;
                    visualspell = SPELL_FR_VISUAL_RAMPAGE;
                    searchentry = NPC_FROZEN_HEAD_MELEE;
                    searchentry2 = NPC_FROZEN_HEAD_RANGE;
                    break;
                default:
                    break;
                }
                std::list<Creature*>myheadlist;
                myheadlist.clear();
                GetCreatureListWithEntryInGrid(myheadlist, GetCaster(), searchentry, 150.0f);
                GetCreatureListWithEntryInGrid(myheadlist, GetCaster(), searchentry2, 150.0f);

                float misc = aurEff->GetAmount();                                      //percent damage
                float mod = myheadlist.size() > 1 ? misc*myheadlist.size() - misc : 0; //increase percent from head count
                SpellInfo const* spell = sSpellMgr->GetSpellInfo(spellentry);
                float basepoint = spell->GetEffect(0)->BasePoints;                     //base damage from spell
                float dmgmod = (basepoint*(mod / 100));                                //calculate damage from modifier percent
                float dmg = basepoint + dmgmod;                                        //update damage

                std::list<Player*>pllist;
                pllist.clear();
                GetPlayerListInGrid(pllist, GetCaster(), 150.0f);
                std::list<Player*>::iterator itr = pllist.begin();
                std::advance(itr, urand(0, pllist.size() - 1));

                GetCaster()->CastSpell(*itr, visualspell, true);
                GetCaster()->CastCustomSpell(spellentry, SPELLVALUE_BASE_POINT0, dmg, GetCaster(), true);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_megaera_rampage_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_megaera_rampage_AuraScript();
    }
};

void AddSC_boss_megaera()
{
    new npc_megaera();
    new npc_megaera_head();
    new npc_cinders();
    new npc_acid_rain();
    new npc_torrent_of_ice();
    new npc_icy_ground();
    new spell_cinders();
    new spell_acid_rain();
    new spell_torrent_of_ice();
    new spell_megaera_rampage();
}
