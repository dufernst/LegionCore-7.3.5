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

enum eSpells
{
    SPELL_BERSERK               = 26662,

    //Shamans
    SPELL_BLOODLUST             = 144302,
    SPELL_POISONMIST_TOTEM      = 144288,
    SPELL_FOULSTREAM_TOTEM      = 144289,
    SPELL_ASHFLARE_TOTEM        = 144290,

    //Kardris
    SPELL_FROSTSTORM_BOLT       = 144214,
    //95pct HM
    SPELL_IRON_PRISON           = 144330, 
    SPELL_IRON_PRISON_DMG       = 144331,
    //85pct
    SPELL_TOXIC_STORM_SUM       = 144005,
    SPELL_TOXIC_STORM_TR_AURA   = 144006,
    SPELL_TOXIC_TORNADO_SUM     = 144019,
    SPELL_TOXIC_TORNADO_TR_AURA = 144029,
    //65pct
    SPELL_FOUL_GEYSER           = 143990,
    //50pct
    SPELL_FALLING_ASH_SUM       = 143973,
    SPELL_FALLING_ASH_DMG       = 143987,
    SPELL_FALLING_ASH_COSMETIC  = 143986,
    SPELL_FALLING_ASH_DEST_V    = 149016, //conveyer change beam visual(Balckfuse)

    //Haromm
    SPELL_FROSTSTORM_STRIKE     = 144215,
    //95pct HM
    SPELL_IRON_TOMB             = 144328,
    SPELL_IRON_TOMB_SUM         = 144329, 
    SPELL_IRON_TOMB_DMG         = 144334,
    //85pct
    SPELL_TOXIC_MIST            = 144089,
    //65pct
    SPELL_FOUL_STREAM           = 144090,
    //50pct
    SPELL_ASH_ELEMENTAL_SPAWN   = 144222,
    SPELL_ASHEN_WALL            = 144070,

    //Mount
    SPELL_SWIPE                 = 144303,
    SPELL_REND                  = 144304,

    //Other Creature
    SPELL_FOULNESS              = 144064,
    SPELL_RESISTANCE_TOTEM      = 145730,
    SPELL_RESISTANCE_TOTEM_SUM  = 145732,
    SPELL_TOXICITY              = 144107,
};

enum CreatureText
{
    SAY_PULL                    = 0,
    SAY_FIRSTATTACK             = 1,
    SAY_POISONMIST_TOTEM        = 2,
    SAY_ASHFLARE_TOTEM          = 3,
    SAY_BLOODLUST               = 4,
    SAY_FOULSTREAM_TOTEM        = 5,
};

enum sEvents
{
    //Shamans
    //Kardris
    EVENT_FROSTSTORM_BOLT       = 1,
    EVENT_TOXIC_STORM           = 2,
    EVENT_FOUL_GEYSER           = 3,
    EVENT_FALLING_ASH           = 4,
    //HM
    EVENT_IRON_PRISON           = 5,

    //Haromm
    EVENT_FROSTSTORM_STRIKE     = 6,
    EVENT_TOXIC_MIST            = 7,
    EVENT_FOUL_STREAM           = 8,
    EVENT_ASHEN_WALL            = 9,
    //HM
    EVENT_IRON_TOMB             = 10,

    //Mounts
    EVENT_SWIPE                 = 11,
    EVENT_REND                  = 12,

    EVENT_SUMMON_TORNADO        = 13,
    EVENT_ACTIVE                = 14,
    EVENT_DESPAWN               = 15,

    EVENT_BERSERK               = 16,
    EVENT_FREEDOM               = 17,
};

enum Data
{
    DATA_GET_PULL_STATE         = 1,
};

enum sActions
{
    ACTION_FREEDOM              = 1,
};

Position const spawnpos[4] =
{
    {1668.15f, -4354.63f, 26.3788f, 2.87635f},
    {1658.76f, -4337.2f,  26.3985f, 4.43536f},
    {1605.02f, -4384.74f, 20.4198f,  3.6096f},
    {1598.41f, -4372.17f, 21.1200f,  3.6896f},
};

struct  SpecialModifier
{
    float dist, ang;
};

static SpecialModifier mod[] =
{
    { 5.0f,  1.570796326795f },
    { 10.0f, 1.570796326795f },
    { 15.0f, 1.570796326795f },
    { 20.0f, 1.570796326795f },
    { 5.0f,  M_PI + 1.570796326795f },
    { 10.0f, M_PI + 1.570796326795f },
    { 15.0f, M_PI + 1.570796326795f },
    { 20.0f, M_PI + 1.570796326795f },
};

float const minx = 1483.13f;
float const maxx = 1744.00f;
float const miny = -4417.83f;
class boss_korkron_dark_shaman : public CreatureScript
{
public:
    boss_korkron_dark_shaman() : CreatureScript("boss_korkron_dark_shaman") {}

    struct boss_korkron_dark_shamanAI : public ScriptedAI
    {
        boss_korkron_dark_shamanAI(Creature* creature) : ScriptedAI(creature), summon(me)
        {
            instance = creature->GetInstanceScript();
            firstpull = true;
        }

        InstanceScript* instance;
        SummonList summon;
        EventMap events;
        uint32 evadecheck;
        //uint8 nextpct;
        bool firstpull, firstattack;
        bool stage, stage2, stage3, stage4, stage5;
        
        void Reset()
        {
            events.Reset();
            summon.DespawnAll();
            DespawnAllSummons();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            instance->SetBossState(DATA_KORKRON_D_SHAMAN, NOT_STARTED);
            if (firstpull)
                SummonAndSeatOnMount(me->GetEntry());
            /*if (me->GetMap()->IsHeroic())
                nextpct = 95;
            else
                nextpct = 85;*/
            firstattack = false;
            stage = false;
            stage2 = false;
            stage3 = false;
            stage4 = false;
            stage5 = false;
            evadecheck = 0;
        }

        Creature* GetOtherShaman()
        {
            if (instance)
                if (Creature* oshaman = me->GetCreature(*me, instance->GetGuidData(me->GetEntry() == NPC_WAVEBINDER_KARDRIS ? NPC_EARTHBREAKER_HAROMM : NPC_WAVEBINDER_KARDRIS)))
                    if (oshaman->isAlive())
                        return oshaman;
            return NULL;
        }

        void DespawnAllSummons() //Despawn special creature and gameobject(summons from triggers)
        {
            std::list<Creature*> list;
            list.clear();
            me->GetCreatureListWithEntryInGrid(list, 71827, 200.0f); //ash elemental
            me->GetCreatureListWithEntryInGrid(list, 71817, 200.0f); //toxic tornado
            me->GetCreatureListWithEntryInGrid(list, NPC_IRON_TOMB, 200.0f);
            if (!list.empty())
                for (std::list<Creature*>::const_iterator itr = list.begin(); itr != list.end(); itr++)
                    (*itr)->DespawnOrUnsummon();

            if (me->GetMap()->IsHeroic())
            {
                std::list<GameObject*>glist;
                glist.clear();
                me->GetGameObjectListWithEntryInGrid(glist, 220864, 200.0f); //iron tomb
                if (!glist.empty())
                    for (std::list<GameObject*>::const_iterator itr = glist.begin(); itr != glist.end(); itr++)
                        (*itr)->Delete();
            }
        }

        void JustReachedHome()
        {
            if (!firstpull)
                SummonAndSeatOnMount(me->GetEntry());
        }

        void SummonAndSeatOnMount(uint32 entry)
        {
            uint32 mauntentry = 0;
            switch (entry)
            {
            case NPC_WAVEBINDER_KARDRIS:
                mauntentry = NPC_BLOODCLAW;
                break;
            case NPC_EARTHBREAKER_HAROMM:
                mauntentry = NPC_DARKFANG;
                break;
            }
            if (Creature* mount = me->SummonCreature(mauntentry, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ()))
            {
                if (!firstpull)
                    mount->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->EnterVehicle(mount->ToUnit(), 0);
            }
            instance->SetData(DATA_CHECK_KDS_RESET_IS_DONE, 0);
        }
        
        void EnterCombat(Unit* who)
        {
            if (instance)
                instance->SetBossState(DATA_KORKRON_D_SHAMAN, IN_PROGRESS);

            if (firstpull)
            {
                firstpull = false;
                switch (me->GetEntry())
                {
                case NPC_WAVEBINDER_KARDRIS:
                    me->SetHomePosition(spawnpos[2]);
                    break;
                case NPC_EARTHBREAKER_HAROMM:
                    me->SetHomePosition(spawnpos[3]);
                    break;
                }
            }
            Talk(SAY_PULL);
            evadecheck = 1500;
            events.RescheduleEvent(EVENT_BERSERK, 600000);
            events.RescheduleEvent(me->GetEntry() == NPC_WAVEBINDER_KARDRIS ? EVENT_FROSTSTORM_BOLT : EVENT_FROSTSTORM_STRIKE, 6000);
        }

        void EnterEvadeMode()
        {
            me->NearTeleportTo(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY(), me->GetHomePosition().GetPositionZ(), me->GetHomePosition().GetOrientation());
            ScriptedAI::EnterEvadeMode();

            if (auto oshaman = GetOtherShaman())
            {
                if (oshaman->isInCombat() && !oshaman->IsInEvadeMode())
                    oshaman->AI()->EnterEvadeMode();
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (attacker != me)
            {
                if (Creature* oshaman = GetOtherShaman())
                {
                    if (damage >= me->GetHealth())
                        oshaman->Kill(oshaman, true);
                    else
                        oshaman->DealDamage(oshaman, damage);
                }
            }

            if (me->GetMap()->IsHeroic() && HealthBelowPct(95) && !stage)
            {
                stage = true;
                SetExtraEvents(85);
            }
            else if (HealthBelowPct(85) && !stage2)
            {
                stage2 = true;
                SetExtraEvents(65);
            }
            else if (HealthBelowPct(65) && !stage3)
            {
                stage3 = true;
                SetExtraEvents(50);
            }
            else if (HealthBelowPct(50) && !stage4)
            {
                stage4 = true;
                SetExtraEvents(25);
            }
            else if (HealthBelowPct(25) && !stage5)
            {
                stage5 = true;
                SetExtraEvents(0);
            }
            //old version
            /*if (Creature* oshaman = GetOtherShaman())
            {
                if (damage >= me->GetHealth())
                    oshaman->Kill(oshaman, true);
                else
                    oshaman->SetHealth(oshaman->GetHealth() - damage);
            }*/
        }
        
        void SetExtraEvents(uint8 phase)
        {
            switch (phase)
            {
            case 85: //95pct HM
            {
                switch (me->GetEntry())
                {
                case NPC_WAVEBINDER_KARDRIS:
                    events.RescheduleEvent(EVENT_IRON_PRISON, 2000);
                    break;
                case NPC_EARTHBREAKER_HAROMM:
                    events.RescheduleEvent(EVENT_IRON_TOMB, 4000);
                    break;
                }
            }
            break;
            case 65: //85pct
            {
                switch (me->GetEntry())
                {
                case NPC_WAVEBINDER_KARDRIS:
                    Talk(SAY_POISONMIST_TOTEM);
                    DoCast(me, SPELL_POISONMIST_TOTEM, true);
                    events.RescheduleEvent(EVENT_TOXIC_STORM, 5000);
                    break;
                case NPC_EARTHBREAKER_HAROMM:
                    events.RescheduleEvent(EVENT_TOXIC_MIST, 2000);
                    break;
                }
            }
            break;
            case 50: //65pct
            {
                switch (me->GetEntry())
                {
                case NPC_WAVEBINDER_KARDRIS:
                    DoCast(me, SPELL_FOULSTREAM_TOTEM, true);
                    events.RescheduleEvent(EVENT_FOUL_GEYSER, 2000);
                    break;
                case NPC_EARTHBREAKER_HAROMM:
                    Talk(SAY_FOULSTREAM_TOTEM);
                    events.RescheduleEvent(EVENT_FOUL_STREAM, 1000);
                    break;
                }
            }
            break;
            case 25: //50pct
            {
                switch (me->GetEntry())
                {
                case NPC_WAVEBINDER_KARDRIS:
                    Talk(SAY_ASHFLARE_TOTEM);
                    DoCast(me, SPELL_ASHFLARE_TOTEM, true);
                    events.RescheduleEvent(EVENT_FALLING_ASH, 2000);
                    break;
                case NPC_EARTHBREAKER_HAROMM:
                    events.RescheduleEvent(EVENT_ASHEN_WALL, 2000);
                    break;
                }
            }
            break;
            case 0: //25pct
                Talk(SAY_BLOODLUST);
                me->AddAura(SPELL_BLOODLUST, me);
                break;
            }
        }
        
        void JustDied(Unit* killer)
        {
            summon.DespawnAll();
            DespawnAllSummons();
            if (killer != me)
            {
                instance->SetBossState(DATA_KORKRON_D_SHAMAN, DONE);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_IRON_PRISON);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TOXIC_MIST);
            }
        }
        
        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (evadecheck)
            {
                if (evadecheck <= diff)
                {
                    if (me->GetPositionX() <= minx || (me->GetPositionX() >= maxx && me->GetPositionY() <= miny))
                    {
                        evadecheck = 0;
                        me->SetReactState(REACT_PASSIVE);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        EnterEvadeMode();
                    }
                    else
                        evadecheck = 1500;
                }
                else
                    evadecheck -= diff;
            }

            //old version
            /*if (HealthBelowPct(nextpct))
            {
                switch (nextpct)
                {
                case 95:
                    nextpct = 85;
                    break;
                case 85:
                    nextpct = 65;
                    break;
                case 65:
                    nextpct = 50;
                    break;
                case 50:
                    nextpct = 25;
                    break;
                case 25:
                    nextpct = 0;
                    break;
                }
                SetExtraEvents(nextpct);
            }*/

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                //Base Events
                //Kardris
                case EVENT_FROSTSTORM_BOLT:
                    if (me->getVictim())
                    {
                        if (!firstattack)
                        {
                            firstattack = true;
                            Talk(SAY_FIRSTATTACK);
                        }
                        DoCastVictim(SPELL_FROSTSTORM_BOLT);
                    }
                    events.RescheduleEvent(EVENT_FROSTSTORM_BOLT, 6000);
                    break;
                //Haromm
                case EVENT_FROSTSTORM_STRIKE:
                    if (me->getVictim())
                    {
                        if (!firstattack)
                        {
                            firstattack = true;
                            Talk(SAY_FIRSTATTACK);
                        }
                        DoCastVictim(SPELL_FROSTSTORM_STRIKE);
                    }
                    events.RescheduleEvent(EVENT_FROSTSTORM_STRIKE, 6000);
                    break;
                //Extra Events 95 pct HM
                //Kardris
                case EVENT_IRON_PRISON:
                    //targets push and filter in script
                    if (Player* pl = me->FindNearestPlayer(60.0f, true))
                        DoCast(pl, SPELL_IRON_PRISON);
                    events.RescheduleEvent(EVENT_IRON_PRISON, 30000);
                    break;
                //Haromm
                case EVENT_IRON_TOMB:
                    //targets push and filter in script
                    if (Player* pl = me->FindNearestPlayer(60.0f, true))
                        DoCast(pl, SPELL_IRON_TOMB);
                    events.RescheduleEvent(EVENT_IRON_TOMB, 30000);
                    break;
                //Extra Events 85 pct
                //Kardris
                case EVENT_TOXIC_STORM:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 60.0f, true))
                        DoCast(target, SPELL_TOXIC_STORM_SUM);
                    events.RescheduleEvent(EVENT_TOXIC_STORM, 30000);
                    break;
                //Haromm
                case EVENT_TOXIC_MIST:
                {
                    bool havetarget = false;
                    uint8 num = 0;
                    std::list<Player*>pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 80.0f);
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                        {
                            if (!(*itr)->isInTankSpec())
                            {
                                if (!(*itr)->HasAura(SPELL_TOXIC_MIST))
                                {
                                    if (!havetarget)
                                        havetarget = true;
                                    (*itr)->AddAura(SPELL_TOXIC_MIST, *itr);
                                    num++;
                                    if (num == 2)
                                        break;
                                }
                            }
                        }
                        if (!havetarget)
                        {
                            for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                            {
                                if (!(*itr)->HasAura(SPELL_TOXIC_MIST))
                                {
                                    (*itr)->AddAura(SPELL_TOXIC_MIST, *itr);
                                    num++;
                                    if (num == 2)
                                        break;
                                }
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_TOXIC_MIST, 30000);
                    break;
                }
                //Extra events 65pct
                //Kardris
                case EVENT_FOUL_GEYSER:
                    if (me->getVictim())
                        DoCastVictim(SPELL_FOUL_GEYSER);
                    events.RescheduleEvent(EVENT_FOUL_GEYSER, 30000);
                    break;
                //Haromm
                case EVENT_FOUL_STREAM:
                {
                    std::list<Player*>pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 50.0f);
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                        {
                            if (!(*itr)->HasAura(SPELL_TOXIC_MIST))
                            {
                                DoCast(*itr, SPELL_FOUL_STREAM);
                                break;
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_FOUL_STREAM, 30000);
                    break;
                }
                //Extra events 50pct
                //Kardris
                case EVENT_FALLING_ASH:
                    if (Unit* target = me->getVictim())
                    {
                        Position pos;
                        target->GetPosition(&pos);
                        me->SummonCreature(NPC_FALLING_ASH_GROUND_STALKER, pos, TEMPSUMMON_TIMED_DESPAWN, 17000);
                    }
                    events.RescheduleEvent(EVENT_FALLING_ASH, 30000);
                    break;
                //Haromm
                case EVENT_ASHEN_WALL:
                    if (me->getVictim())
                        DoCast(me->getVictim(), SPELL_ASHEN_WALL);
                    events.RescheduleEvent(EVENT_ASHEN_WALL, 30000);
                    break;
                case EVENT_BERSERK:
                    DoCast(me, SPELL_BERSERK);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }

        void JustSummoned(Creature* sum)
        {
            summon.Summon(sum);
            switch (sum->GetEntry())
            {
            //Totems
            case 71915:
            case 71916:
            case 71917:
                sum->SetReactState(REACT_PASSIVE);
                sum->AttackStop();
                break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_GET_PULL_STATE)
                return firstpull ? 1 : 0;
            return 0;
        }
    };
    
    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_korkron_dark_shamanAI(creature);
    }
};

class npc_wolf_maunt : public CreatureScript
{
public:
    npc_wolf_maunt() : CreatureScript("npc_wolf_maunt") {}

    struct npc_wolf_mauntAI : public ScriptedAI
    {
        npc_wolf_mauntAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            SetHomePos();
            me->GetMotionMaster()->MoveTargetedHome();
            events.Reset();
        }

        void EnterCombat(Unit* who)
        {
            if (instance)
            {
                if (Creature* omount = me->GetCreature(*me, instance->GetGuidData(me->GetEntry() == NPC_BLOODCLAW ? NPC_DARKFANG : NPC_BLOODCLAW)))
                    if (!omount->isInCombat())
                        omount->AI()->DoZoneInCombat(omount, 150.0f);
                CallAndDismountShamans();
                events.RescheduleEvent(EVENT_SWIPE, 7000);
                events.RescheduleEvent(EVENT_REND,  13000);
            }
        }

        void CallAndDismountShamans()
        {
            if (Vehicle* _me = me->GetVehicleKit())
            {
                if (Unit* p = _me->GetPassenger(0))
                {
                    p->_ExitVehicle();
                    p->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    p->ToCreature()->SetReactState(REACT_AGGRESSIVE);
                    p->ToCreature()->AI()->DoZoneInCombat(p->ToCreature(), 150.0f);
                }
            }
        }

        void SetHomePos()
        {
            if (me->ToTempSummon() && me->ToTempSummon()->GetSummoner())
            {
                if (Creature* ds = me->ToTempSummon()->GetSummoner()->ToCreature())
                {
                    switch (me->GetEntry())
                    {
                    case NPC_BLOODCLAW:
                        if (ds->AI()->GetData(DATA_GET_PULL_STATE))
                            me->SetHomePosition(spawnpos[0]);
                        else
                            me->SetHomePosition(spawnpos[2]);
                        break;
                    case NPC_DARKFANG:
                        if (ds->AI()->GetData(DATA_GET_PULL_STATE))
                            me->SetHomePosition(spawnpos[1]);
                        else
                            me->SetHomePosition(spawnpos[3]);
                        break;
                    }
                }
            }
        }
        
        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_SWIPE:
                    if (me->getVictim())
                        DoCastVictim(SPELL_SWIPE, true);
                    events.RescheduleEvent(EVENT_SWIPE, 7000);
                    break;
                case EVENT_REND:
                    if (me->getVictim())
                        DoCastVictim(SPELL_REND, true);
                    events.RescheduleEvent(EVENT_REND, 4000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_wolf_mauntAI(creature);
    }
}; 

//71801
class npc_toxic_storm : public CreatureScript
{
public:
    npc_toxic_storm() : CreatureScript("npc_toxic_storm") {}

    struct npc_toxic_stormAI : public ScriptedAI
    {
        npc_toxic_stormAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            DoCast(me, SPELL_TOXIC_STORM_TR_AURA, true);
        }

        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            events.RescheduleEvent(EVENT_SUMMON_TORNADO, 12000);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_SUMMON_TORNADO)
                {
                    float ang = float(urand(0, 6));
                    me->SetOrientation(ang);
                    DoCast(me, SPELL_TOXIC_TORNADO_SUM);
                    events.RescheduleEvent(EVENT_SUMMON_TORNADO, 12000);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_toxic_stormAI(creature);
    }
};

//71817
class npc_toxic_tornado : public CreatureScript
{
public:
    npc_toxic_tornado() : CreatureScript("npc_toxic_tornado") {}

    struct npc_toxic_tornadoAI : public ScriptedAI
    {
        npc_toxic_tornadoAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_HOVER);
            DoCast(me, SPELL_TOXIC_TORNADO_TR_AURA, true);
        }

        InstanceScript* instance;

        void Reset()
        {
            float x = 0, y = 0;
            GetPositionWithDistInOrientation(me, 75.0f, me->GetOrientation(), x, y);
            me->GetMotionMaster()->MoveCharge(x, y, me->GetPositionZ(), 3.0f);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_toxic_tornadoAI(creature);
    }
};

//71825
class npc_foul_slime : public CreatureScript
{
public:
    npc_foul_slime() : CreatureScript("npc_foul_slime") {}

    struct npc_foul_slimeAI : public ScriptedAI
    {
        npc_foul_slimeAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        void Reset(){}
        
        void IsSummonedBy(Unit* summoner)
        {
            DoCast(me, SPELL_FOULNESS);
            DoZoneInCombat(me, 150.0f);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_foul_slimeAI(creature);
    }
};

//71827
class npc_ash_elemental : public CreatureScript
{
public:
    npc_ash_elemental() : CreatureScript("npc_ash_elemental") {}

    struct npc_ash_elementalAI : public ScriptedAI
    {
        npc_ash_elementalAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetVisible(false);
        }

        InstanceScript* instance;
        EventMap events;

        void Reset(){}

        void IsSummonedBy(Unit* summoner)
        {
            if (!summoner->ToCreature())
            {
                float x, y;
                for (uint8 n = 0; n < 8; n++)
                {
                    x = 0, y = 0;
                    float ang = me->GetOrientation() + mod[n].ang;
                    GetPositionWithDistInOrientation(me, mod[n].dist, ang, x, y);
                    me->SummonCreature(NPC_ASH_ELEMENTAL, x, y, me->GetPositionZ() + 5.0f, me->GetOrientation());
                }
            }
            else
                me->GetMotionMaster()->MoveFall();
            events.RescheduleEvent(EVENT_ACTIVE, 1250);
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_ACTIVE)
                {
                    me->SetVisible(true);
                    DoCast(me, SPELL_ASH_ELEMENTAL_SPAWN, true);
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat(me, 150.0f);
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ash_elementalAI(creature);
    }
};

//71941
class npc_iron_tomb : public CreatureScript
{
public:
    npc_iron_tomb() : CreatureScript("npc_iron_tomb") {}

    struct npc_iron_tombAI : public ScriptedAI
    {
        npc_iron_tombAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }

        InstanceScript* instance;

        void Reset(){}

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void IsSummonedBy(Unit* summoner)
        {
            DoCast(me, SPELL_IRON_TOMB_DMG);
        }

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_iron_tombAI(creature);
    }
};

//72640
class npc_wildhammer_shaman : public CreatureScript
{
public:
    npc_wildhammer_shaman() : CreatureScript("npc_wildhammer_shaman") {}

    struct npc_wildhammer_shamanAI : public ScriptedAI
    {
        npc_wildhammer_shamanAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        InstanceScript* instance;
        EventMap events;
        
        void Reset(){}

        void DoAction(int32 const action)
        {
            if (action == ACTION_FREEDOM)
                events.RescheduleEvent(EVENT_FREEDOM, 2000);
        }

        void JustSummoned(Creature* sum)
        {
            if (sum->GetEntry() == NPC_RESISTANCE_TOTEM)
            {
                sum->SetReactState(REACT_PASSIVE);
                sum->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                sum->setFaction(190);
                if (Player* pl = sum->FindNearestPlayer(80.0f, true))
                    sum->CastSpell(pl, SPELL_RESISTANCE_TOTEM, true);
                me->DespawnOrUnsummon();
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE)
                DoCast(me, SPELL_RESISTANCE_TOTEM_SUM);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_FREEDOM)
                    me->GetMotionMaster()->MoveCharge(1576.97f, -4352.48f, 21.0959f, 2.0f, 1);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_wildhammer_shamanAI(creature);
    }
};

//90007
class npc_falling_ash_ground_stalker : public CreatureScript
{
public:
    npc_falling_ash_ground_stalker() : CreatureScript("npc_falling_ash_ground_stalker") {}

    struct npc_falling_ash_ground_stalkerAI : public ScriptedAI
    {
        npc_falling_ash_ground_stalkerAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        InstanceScript* instance;

        void Reset()
        {
            me->SetFloatValue(OBJECT_FIELD_SCALE, 3.0f);
            me->AddAura(SPELL_FALLING_ASH_DEST_V, me);
            if (Aura* aura = me->GetAura(SPELL_FALLING_ASH_DEST_V))
                aura->SetDuration(15000);
            float x = me->GetPositionX();
            float y = me->GetPositionY();
            float z = me->GetPositionZ() + 50.0f;
            if (me->ToTempSummon())
                if (Unit* kardris = me->ToTempSummon()->GetSummoner())
                    kardris->SummonCreature(NPC_FALLING_ASH, x, y, z);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_falling_ash_ground_stalkerAI(creature);
    }
};

//71789
class npc_falling_ash : public CreatureScript
{
public:
    npc_falling_ash() : CreatureScript("npc_falling_ash") {}

    struct npc_falling_ashAI : public ScriptedAI
    {
        npc_falling_ashAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        InstanceScript* instance;

        void Reset()
        {
            DoCast(me, SPELL_FALLING_ASH_COSMETIC, true);
            if (Creature* fags = me->FindNearestCreature(NPC_FALLING_ASH_GROUND_STALKER, 100.0f, true))
                me->GetMotionMaster()->MoveCharge(fags->GetPositionX(), fags->GetPositionY(), fags->GetPositionZ(), 3.2f, 1);
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE)
            {
                DoCast(me, SPELL_FALLING_ASH_DMG, true);
                me->DespawnOrUnsummon();
            }
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_falling_ashAI(creature);
    }
};

//144070
class spell_asher_wall : public SpellScriptLoader
{
public:
    spell_asher_wall() : SpellScriptLoader("spell_asher_wall") { }

    class spell_asher_wall_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_asher_wall_SpellScript);

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            if (GetHitUnit())
                GetHitUnit()->SummonCreature(NPC_ASH_ELEMENTAL, GetHitUnit()->GetPositionX(), GetHitUnit()->GetPositionY(), GetHitUnit()->GetPositionZ(), GetHitUnit()->GetOrientation());
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_asher_wall_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_asher_wall_SpellScript();
    }
};

class IronTombRangeFilter
{
public:
    IronTombRangeFilter(Unit* caster) : _caster(caster){}

    bool operator()(WorldObject* unit) const
    {
        if (Player* pl = unit->ToPlayer())
            if (_caster->GetDistance(pl) < 15.0f)
                return true;
        return false;
    }
private:
    Unit* _caster;
};

//144328
class spell_iron_tomb : public SpellScriptLoader
{
public:
    spell_iron_tomb() : SpellScriptLoader("spell_iron_tomb") { }

    class spell_iron_tomb_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_iron_tomb_SpellScript);

        void _FilterTarget(std::list<WorldObject*>&targets)
        {
            if (GetCaster())
            {
                bool havetargetinrange = false;
                if (!targets.empty())
                {
                    for (std::list<WorldObject*>::const_iterator itr = targets.begin(); itr != targets.end(); itr++)
                    {
                        if (GetCaster()->GetDistance(*itr) >= 15.0f)
                        {
                            havetargetinrange = true;
                            break;
                        }
                    }
                }

                if (havetargetinrange)
                    targets.remove_if(IronTombRangeFilter(GetCaster()));

                if (targets.size() > 3)
                    targets.resize(3);
            }
        }

        void _HandleHit()
        {
            if (GetHitUnit())
                GetHitUnit()->CastSpell(GetHitUnit(), SPELL_IRON_TOMB_SUM, true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_iron_tomb_SpellScript::_FilterTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnHit += SpellHitFn(spell_iron_tomb_SpellScript::_HandleHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_iron_tomb_SpellScript();
    }
};

class IronPrisonFilter
{
public:
    bool operator()(WorldObject* unit) const
    {
        if (Player* pl = unit->ToPlayer())
        {
            if (pl->isInTankSpec())
                return true;

            if (pl->HasAura(SPELL_IRON_PRISON))
                return true;
        }
        return false;
    }
};

//144330
class spell_iron_prison : public SpellScriptLoader
{
public:
    spell_iron_prison() : SpellScriptLoader("spell_iron_prison") { }

    class spell_iron_prison_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_iron_prison_SpellScript);

        void _FilterTarget(std::list<WorldObject*>&targets)
        {
            targets.remove_if(IronPrisonFilter());
            if (targets.size() > 2)
                targets.resize(2);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_iron_prison_SpellScript::_FilterTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    class spell_iron_prison_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_iron_prison_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetTarget())
                GetTarget()->CastSpell(GetTarget(), SPELL_IRON_PRISON_DMG, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_iron_prison_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_iron_prison_SpellScript();
    }

    AuraScript* GetAuraScript() const
    {
        return new spell_iron_prison_AuraScript();
    }
};

//145690
class spell_kds_unlock : public SpellScriptLoader
{
public:
    spell_kds_unlock() : SpellScriptLoader("spell_kds_unlock") { }

    class spell_kds_unlock_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_kds_unlock_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster())
                if (GameObject* go = GetCaster()->FindNearestGameObject(GO_KORKRON_CAGE, 12.0f))
                    if (Creature* wshaman = go->FindNearestCreature(NPC_WILDHAMMER_SHAMAN, 5.0f, true))
                        wshaman->AI()->DoAction(ACTION_FREEDOM);
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_kds_unlock_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_kds_unlock_SpellScript();
    }
};

//144331
class spell_iron_prison_dmg : public SpellScriptLoader
{
public:
    spell_iron_prison_dmg() : SpellScriptLoader("spell_iron_prison_dmg") { }

    class spell_iron_prison_dmg_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_iron_prison_dmg_SpellScript);

        void _HandleHit()
        {
            if (GetHitUnit())
                if (GetHitUnit()->HasAura(1022))//bubble
                    SetHitDamage(0);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_iron_prison_dmg_SpellScript::_HandleHit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_iron_prison_dmg_SpellScript();
    }
};

//144089
class spell_toxic_mist : public SpellScriptLoader
{
public:
    spell_toxic_mist() : SpellScriptLoader("spell_toxic_mist") { }

    class spell_toxic_mist_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_toxic_mist_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTarget() && GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
                GetTarget()->RemoveAurasDueToSpell(SPELL_TOXICITY);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_toxic_mist_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_toxic_mist_AuraScript();
    }
};

void AddSC_boss_korkron_dark_shaman()
{
    new boss_korkron_dark_shaman();
    new npc_wolf_maunt();
    new npc_toxic_storm();
    new npc_toxic_tornado();
    new npc_foul_slime();
    new npc_ash_elemental();
    new npc_iron_tomb();
    new npc_wildhammer_shaman();
    new npc_falling_ash_ground_stalker();
    new npc_falling_ash();
    new spell_asher_wall();
    new spell_iron_tomb();
    new spell_iron_prison();
    new spell_kds_unlock();
    new spell_iron_prison_dmg();
    new spell_toxic_mist();
}
