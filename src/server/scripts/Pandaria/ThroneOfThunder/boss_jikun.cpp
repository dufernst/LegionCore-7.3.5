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

#include "PlayerDefines.h"
#include "throne_of_thunder.h"

enum eSpells
{
    //JiKun
    SPELL_CAW_SEARCHER            = 138923,
    SPELL_CAW_DMG                 = 138926,
    SPELL_QUILLS                  = 134380,
    SPELL_TALON_RAKE              = 134366,
    SPELL_INFECTED_TALONS         = 140092,
    SPELL_DOWN_DRAFT_AT           = 134370,
    SPELL_FEED_YOUNG              = 137528, //main spell

    //Fly buffs and spells
    SPELL_LESSON_OF_ICARUS        = 140571,
    SPELL_DAEDALIAN_WINGS         = 134339, //ovveride spells aura
    SPELL_FEATHER_AT              = 134338,
    SPELL_JUMP_TO_B_PLATFORM      = 138360,
    SPELL_JI_KUN_FEATHER_AURA     = 140014,
    SPELL_JI_KUN_FEATHER_USE_EF   = 140013,
    SPELL_FLY                     = 133755,

    //Incubate
    SPELL_INCUBATE_TARGET_AURA    = 134347,
    SPELL_INCUBATE_TARGET_AURA_2  = 134335,

    //Feed for nest
    SPELL_PRIMAL_NUTRIMENT        = 140741, //buff if catch slimed
    SPELL_PRIMAL_NUTRIMENT_TR_S   = 112879,
    SPELL_FEED_POOL_SPAWN_N       = 139284, //pool in nest visual

    //Feed for platform
    SPELL_FEED_POOL_SPAWN_P       = 138854, //pool on platform visual
    SPELL_FEED_POOL_DMG           = 138319,
    SPELL_SLIMED_DMG              = 134256,
    SPELL_SLIMED_DEBUFF           = 138309,

    //Young Hatchling
    SPELL_CHEEP_LOW               = 139296,
    SPELL_EAT_CHANNEL             = 134321,
    SPELL_MORPH                   = 134322,
    //Morph Hatchling
    SPELL_LAY_EGG                 = 134367,

    //Juvenile
    SPELL_CHEEP_HIGHT             = 140129,

    //Fallcatcher
    SPELL_SAFE_NET_TRIGGER        = 136524, //summon fallcatcher
    SPELL_GENTLE_YET_FIRM         = 139168,
    SPELL_CAST_FILTER             = 141062,
    SPELL_PARACHUTE               = 45472,
    SPELL_PARACHUTE_BUFF          = 44795,

    //Other
    SPELL_SPAWN_FEED              = 138918, //change color
};

enum eEvents
{
    EVENT_CAW                     = 1,
    EVENT_QUILLS                  = 2,
    EVENT_TALON_RAKE              = 3,
    EVENT_ACTIVE_NEST             = 4,
    EVENT_FALLCATCHER_IN_POS      = 5,
    EVENT_DOWN_DRAFT              = 6,
    EVENT_FEED_YOUNG              = 7,
    EVENT_FIND_PLAYER             = 8,
    EVENT_CHEEP                   = 10,
    EVENT_TAKEOFF                 = 11,
    EVENT_TAKEOFF_2               = 12,
    EVENT_ENTERCOMBAT             = 13,
    EVENT_ENTERCOMBAT_2           = 14,
    EVENT_TO_PATROL               = 15,
    EVENT_MOVE_TO_DEST_POS        = 16,
    EVENT_LAY_EGG                 = 17,
    EVENT_TAKEOFF_3               = 18,
    EVENT_MOVE_TO_DEST_POS_2      = 19,
    EVENT_EAT                     = 20,
    EVENT_BATTLE_RESPAWN_NEST     = 21,
    EVENT_CHECK_PROGRESS          = 22,
};

class boss_jikun : public CreatureScript
{
public:
    boss_jikun() : CreatureScript("boss_jikun") {}

    struct boss_jikunAI : public BossAI
    {
        boss_jikunAI(Creature* creature) : BossAI(creature, DATA_JI_KUN)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_DEFENSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        InstanceScript* instance;
        uint32 checkmelee;

        void Reset()
        {
            _Reset();
            me->SetFullHealth();
            DespawnAllTriggers();
            RemoveDebuffsFromPlayers();
            checkmelee = 0;
            instance->SetData(DATA_JIKUN_RESET_ALL_NESTS, 0);
        }

        void DespawnAllTriggers()
        {
            std::list<Creature*> list;
            list.clear();
            me->GetCreatureListWithEntryInGrid(list, NPC_FEED_P_POOL, 100.0f);
            if (!list.empty())
                for (std::list<Creature*>::const_iterator itr = list.begin(); itr != list.end(); itr++)
                    (*itr)->DespawnOrUnsummon();

            list.clear();
            me->GetCreatureListWithEntryInGrid(list, NPC_JUVENILE, 150.0f);
            me->GetCreatureListWithEntryInGrid(list, NPC_JUVENILE_FROM_F_EGG, 150.0f);
            if (!list.empty())
                for (std::list<Creature*>::const_iterator itr = list.begin(); itr != list.end(); itr++)
                    (*itr)->DespawnOrUnsummon();
        }

        void RemoveDebuffsFromPlayers()
        {
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TALON_RAKE);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FEED_POOL_DMG);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SLIMED_DMG);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_SLIMED_DEBUFF);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PRIMAL_NUTRIMENT);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_PRIMAL_NUTRIMENT_TR_S);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_DAEDALIAN_WINGS);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FLY);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_INFECTED_TALONS);
        }

        void EnterCombat(Unit* who)
        {
            _EnterCombat();
            checkmelee = 4000;
            events.RescheduleEvent(EVENT_ACTIVE_NEST, 1000);
            events.RescheduleEvent(EVENT_CHECK_PROGRESS, 4000);
            events.RescheduleEvent(EVENT_CAW, 14000);
            events.RescheduleEvent(EVENT_TALON_RAKE, 20000);
            events.RescheduleEvent(EVENT_QUILLS, 42000);
            events.RescheduleEvent(EVENT_DOWN_DRAFT, 97000);
        }

        void JustDied(Unit* /*killer*/)
        {
            _JustDied();
            DespawnAllTriggers();
            RemoveDebuffsFromPlayers();
            instance->SetData(DATA_JIKUN_RESET_ALL_NESTS, 0);
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
                switch (eventId)
                {
                case EVENT_CHECK_PROGRESS:
                    if (instance->GetBossState(DATA_MEGAERA) != DONE)
                        ScriptedAI::EnterEvadeMode();
                    break;
                case EVENT_ACTIVE_NEST:
                    instance->SetData(DATA_ACTIVE_NEXT_NEST, 0);
                    events.RescheduleEvent(EVENT_FEED_YOUNG, 2000);
                    events.RescheduleEvent(EVENT_ACTIVE_NEST, 30000);
                    break;
                case EVENT_FEED_YOUNG:
                    DoCast(me, SPELL_FEED_YOUNG);
                    break;
                case EVENT_TALON_RAKE:
                    if (me->getVictim())
                    {
                        if (me->getVictim()->GetTypeId() == TYPEID_PLAYER)
                        {
                            uint8 pos = urand(0, 1);
                            switch (pos)
                            {
                            case 0:
                                DoCast(me->getVictim(), SPELL_TALON_RAKE);
                                break;
                            case 1:
                                DoCast(me->getVictim(), SPELL_INFECTED_TALONS);
                                break;
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_TALON_RAKE, 20000);
                    break;
                case EVENT_QUILLS:
                    DoCast(me, SPELL_QUILLS);
                    events.RescheduleEvent(EVENT_QUILLS, 62500);
                    break;
                case EVENT_CAW:
                    DoCast(me, SPELL_CAW_SEARCHER);
                    events.RescheduleEvent(EVENT_CAW, 20000);
                    break;
                case EVENT_DOWN_DRAFT:
                    checkmelee = 12000;
                    DoCast(me, SPELL_DOWN_DRAFT_AT);
                    events.RescheduleEvent(EVENT_DOWN_DRAFT, 97000);
                    break;
                }
            }

            if (checkmelee <= diff)
            {
                if (!me->HasUnitState(UNIT_STATE_CASTING))
                    if (me->getVictim() && !me->IsWithinMeleeRange(me->getVictim()))
                        DoCast(me, SPELL_QUILLS);
                checkmelee = 4000;
            }
            else
                checkmelee -= diff;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_jikunAI(creature);
    }
};

//69626
class npc_incubater : public CreatureScript
{
public:
    npc_incubater() : CreatureScript("npc_incubater") {}

    struct npc_incubaterAI : public ScriptedAI
    {
        npc_incubaterAI(Creature* creature) : ScriptedAI(creature), summon(me)
        {
            pInstance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        InstanceScript* pInstance;
        EventMap events;
        SummonList summon;

        void Reset(){}

        void JustSummoned(Creature* sum)
        {
            summon.Summon(sum);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
            case DATA_ACTIVE_NEST:
            {
                DoCast(me, SPELL_INCUBATE_ZONE, true);
                std::list<Creature*> egglist;
                egglist.clear();
                uint32 incubatevisualpellid = 0;
                uint32 eggentry = 0;
                if (me->GetPositionZ() >= 37.0f)
                {
                    incubatevisualpellid = SPELL_INCUBATE_TARGET_AURA_2;
                    eggentry = NPC_MATURE_EGG_OF_JIKUN;
                }
                else
                {
                    incubatevisualpellid = SPELL_INCUBATE_TARGET_AURA;
                    eggentry = NPC_YOUNG_EGG_OF_JIKUN;
                    events.RescheduleEvent(EVENT_FEED_YOUNG, 11000);
                }
                GetCreatureListWithEntryInGrid(egglist, me, eggentry, 20.0f);
                uint8 maxsize = me->GetMap()->Is25ManRaid() ? 5 : 4;
                if (egglist.size() > maxsize)
                    egglist.resize(maxsize);
                if (!egglist.empty())
                {
                    for (std::list<Creature*>::const_iterator itr = egglist.begin(); itr != egglist.end(); itr++)
                    {
                        (*itr)->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        (*itr)->CastSpell(*itr, incubatevisualpellid, true);
                    }
                }
                break;
            }
            case DATA_RESET_NEST:
            {
                events.Reset();
                summon.DespawnAll();
                me->RemoveAurasDueToSpell(SPELL_INCUBATE_ZONE);
                std::list<Creature*> egglist;
                egglist.clear();
                GetCreatureListWithEntryInGrid(egglist, me, NPC_YOUNG_EGG_OF_JIKUN, 40.0f);
                GetCreatureListWithEntryInGrid(egglist, me, NPC_MATURE_EGG_OF_JIKUN, 40.0f);
                GetCreatureListWithEntryInGrid(egglist, me, NPC_JIKUN_FLEDGLING_EGG, 40.0f);
                if (!egglist.empty())
                {
                    for (std::list<Creature*>::const_iterator itr = egglist.begin(); itr != egglist.end(); itr++)
                    {
                        if (!(*itr)->isAlive())
                            (*itr)->Respawn();
                        else
                        {
                            (*itr)->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                            (*itr)->RemoveAurasDueToSpell(SPELL_INCUBATE_TARGET_AURA);
                        }
                    }
                }

                egglist.clear();
                GetCreatureListWithEntryInGrid(egglist, me, NPC_FEED_NEST_POOL, 40.0f);
                if (!egglist.empty())
                    for (std::list<Creature*>::const_iterator itr = egglist.begin(); itr != egglist.end(); itr++)
                        (*itr)->DespawnOrUnsummon();

                std::list<AreaTrigger*> atlist;
                atlist.clear();
                me->GetAreaTriggersWithEntryInRange(atlist, 4628, me->GetGUID(), 25.0f);
                if (!atlist.empty())
                    for (std::list<AreaTrigger*>::const_iterator itr = atlist.begin(); itr != atlist.end(); itr++)
                        (*itr)->RemoveFromWorld();
            }
            break;
            }
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FEED_YOUNG:
                {
                    //Get hatchlings in nest
                    std::list<Creature*>feedlist;
                    feedlist.clear();
                    GetCreatureListWithEntryInGrid(feedlist, me, NPC_FEED_NEST_POOL, 20.0f);

                    //Get feed in nest
                    std::list<Creature*>hatchlinglist;
                    hatchlinglist.clear();
                    GetCreatureListWithEntryInGrid(hatchlinglist, me, NPC_HATCHLING, 20.0f);

                    //Push this creatures in vector lists for work
                    GuidVector _hatchlinglist;
                    _hatchlinglist.clear();
                    if (!hatchlinglist.empty())
                        for (std::list<Creature*>::const_iterator itr = hatchlinglist.begin(); itr != hatchlinglist.end(); itr++)
                            _hatchlinglist.push_back((*itr)->GetGUID());

                    GuidVector _feedlist;
                    _feedlist.clear();
                    if (!feedlist.empty())
                        for (std::list<Creature*>::const_iterator Itr = feedlist.begin(); Itr != feedlist.end(); Itr++)
                            _feedlist.push_back((*Itr)->GetGUID());


                    if (!_hatchlinglist.empty() && _feedlist.empty()) //if have alive hatchling but not feed
                    {
                        for (GuidVector::const_iterator itr = _hatchlinglist.begin(); itr != _hatchlinglist.end(); itr++)
                            if (Creature* hatchling = me->GetCreature(*me, *itr))
                                hatchling->AI()->SetData(DATA_ENTERCOMBAT, 0);
                    }
                    else if (!_hatchlinglist.empty() && !_feedlist.empty()) //if have alive hatchling and feed
                    {
                        //Set unique feed for every hatchling, if feed not enough, hatchling launch combat
                        for (uint8 n = 0; n < _hatchlinglist.size(); n++)
                        {
                            if (Creature* hatchling = me->GetCreature(*me, _hatchlinglist[n]))
                            {
                                if (n <= _feedlist.size() - 1)
                                {
                                    if (Creature* feed = me->GetCreature(*me, _feedlist[n]))
                                        hatchling->AI()->SetGUID(feed->GetGUID(), 1);
                                }
                                else
                                    hatchling->AI()->SetData(DATA_ENTERCOMBAT, 0);
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_BATTLE_RESPAWN_NEST, 30000);
                    break;
                }
                case EVENT_BATTLE_RESPAWN_NEST:
                {
                    //clear nest (prepare for next activation)
                    std::list<Creature*> egglist;
                    egglist.clear();
                    GetCreatureListWithEntryInGrid(egglist, me, NPC_YOUNG_EGG_OF_JIKUN, 20.0f);
                    if (!egglist.empty())
                    {
                        for (std::list<Creature*>::const_iterator itr = egglist.begin(); itr != egglist.end(); itr++)
                        {
                            if (!(*itr)->isAlive())
                                (*itr)->Respawn();
                            else
                                (*itr)->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        }
                    }

                    egglist.clear();
                    GetCreatureListWithEntryInGrid(egglist, me, NPC_FEED_NEST_POOL, 20.0f);
                    if (!egglist.empty())
                        for (std::list<Creature*>::const_iterator itr = egglist.begin(); itr != egglist.end(); itr++)
                            (*itr)->DespawnOrUnsummon();

                    std::list<AreaTrigger*> atlist;
                    atlist.clear();
                    me->GetAreaTriggersWithEntryInRange(atlist, 4628, me->GetGUID(), 15.0f);
                    if (!atlist.empty())
                        for (std::list<AreaTrigger*>::const_iterator itr = atlist.begin(); itr != atlist.end(); itr++)
                            (*itr)->RemoveFromWorld();
                }
                break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_incubaterAI(creature);
    }
};

//68194
class npc_young_egg_of_jikun : public CreatureScript
{
public:
    npc_young_egg_of_jikun() : CreatureScript("npc_young_egg_of_jikun") {}

    struct npc_young_egg_of_jikunAI : public ScriptedAI
    {
        npc_young_egg_of_jikunAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        InstanceScript* pInstance;

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        void JustDied(Unit* killer)
        {
            me->RemoveAurasDueToSpell(SPELL_INCUBATE_TARGET_AURA);
            if (killer != me)  //egg destroy
            {
                if (Creature* incubate = me->FindNearestCreature(NPC_INCUBATER, 20.0f, true))
                {
                    float x, y;
                    GetPosInRadiusWithRandomOrientation(me, 2.0f, x, y);
                    incubate->CastSpell(x, y, incubate->GetPositionZ(), SPELL_FEATHER_AT, true);
                }
            }
            else               //incubate complete
            {
                if (Creature* incubate = me->FindNearestCreature(NPC_INCUBATER, 20.0f, true))
                {
                    float x, y;
                    GetPosInRadiusWithRandomOrientation(me, 2.0f, x, y);
                    incubate->SummonCreature(NPC_HATCHLING, x, y, me->GetPositionZ() + 1.0f);
                }
            }
        }

        void EnterCombat(Unit* who){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_young_egg_of_jikunAI(creature);
    }
};

//68192
class npc_hatchling : public CreatureScript
{
public:
    npc_hatchling() : CreatureScript("npc_hatchling") { }

    struct npc_hatchlingAI : public ScriptedAI
    {
        npc_hatchlingAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetReactState(REACT_PASSIVE);
            instance = pCreature->GetInstanceScript();
        }
        InstanceScript* instance;
        EventMap events;
        ObjectGuid feedGuid;

        void Reset(){}

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
            case DATA_MORPH:
                DoCast(me, SPELL_MORPH, true);
                events.RescheduleEvent(EVENT_ENTERCOMBAT, 1000);
                break;
            case DATA_ENTERCOMBAT:
                if (Player* player = me->FindNearestPlayer(20.0f, true))
                {
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->Attack(player, true);
                    me->GetMotionMaster()->MoveChase(player);
                }
                events.RescheduleEvent(EVENT_CHEEP, 5000);
                break;
            }
        }

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            if (Creature* feed = me->GetCreature(*me, guid))
            {
                feedGuid = guid;
                me->SetFacingToObject(feed);
                me->GetMotionMaster()->MoveCharge(feed->GetPositionX(), feed->GetPositionY(), feed->GetPositionZ(), 3.0f, 2);
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE)
                events.RescheduleEvent(EVENT_EAT, 250);
        }

        void JustDied(Unit* killer)
        {
            if (Creature* incubate = me->FindNearestCreature(NPC_INCUBATER, 30.0f, true))
                incubate->CastSpell(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), SPELL_FEATHER_AT, true);
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
                case EVENT_CHEEP:
                    if (!me->HasAura(SPELL_MORPH))
                    {
                        if (me->getVictim())
                            DoCastVictim(SPELL_CHEEP_LOW);
                    }
                    else
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                            DoCast(target, SPELL_CHEEP_HIGHT);
                    }
                    events.RescheduleEvent(EVENT_CHEEP, 8000);
                    break;
                case EVENT_EAT:
                    if (Creature* feed = me->GetCreature(*me, feedGuid))
                        DoCast(feed, SPELL_EAT_CHANNEL);
                    break;
                case EVENT_ENTERCOMBAT:
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat(me, 20.0f);
                    events.RescheduleEvent(EVENT_CHEEP, 5000);
                    events.RescheduleEvent(EVENT_LAY_EGG, 12000);
                    break;
                case EVENT_LAY_EGG:
                    DoCast(me, SPELL_LAY_EGG);
                    events.RescheduleEvent(EVENT_LAY_EGG, 24000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_hatchlingAI(pCreature);
    }
};

//70216
class npc_jikun_feed_pool_nest : public CreatureScript
{
public:
    npc_jikun_feed_pool_nest() : CreatureScript("npc_jikun_feed_pool_nest") { }

    struct npc_jikun_feed_pool_nestAI : public ScriptedAI
    {
        npc_jikun_feed_pool_nestAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(11686);
        }

        void Reset()
        {
            DoCast(me, SPELL_FEED_POOL_SPAWN_N, true);
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

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_jikun_feed_pool_nestAI(pCreature);
    }
};

//68202
class npc_jikun_fledgling_egg : public CreatureScript
{
public:
    npc_jikun_fledgling_egg() : CreatureScript("npc_jikun_fledgling_egg") {}

    struct npc_jikun_fledgling_eggAI : public ScriptedAI
    {
        npc_jikun_fledgling_eggAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }

        void Reset()
        {
            DoCast(me, SPELL_INCUBATE_TARGET_AURA, true);
        }

        void JustDied(Unit* killer)
        {
            me->RemoveAurasDueToSpell(SPELL_INCUBATE_TARGET_AURA);
            //incubate complete
            if (killer == me)
                if (Creature* incubate = me->FindNearestCreature(NPC_INCUBATER, 20.0f, true))
                    if (Creature* juvenile = incubate->SummonCreature(NPC_JUVENILE_FROM_F_EGG, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 5.0f))
                        juvenile->AI()->SetData(DATA_TAKEOFF, 2);
            me->DespawnOrUnsummon();
        }

        void EnterCombat(Unit* who){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_jikun_fledgling_eggAI(creature);
    }
};

//69628
class npc_mature_egg_of_jikun : public CreatureScript
{
public:
    npc_mature_egg_of_jikun() : CreatureScript("npc_mature_egg_of_jikun") {}

    struct npc_mature_egg_of_jikunAI : public ScriptedAI
    {
        npc_mature_egg_of_jikunAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        InstanceScript* pInstance;

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        }

        void JustDied(Unit* killer)
        {
            me->RemoveAurasDueToSpell(SPELL_INCUBATE_TARGET_AURA_2);
            if (killer != me)  //egg destroy
            {
                if (Creature* incubate = me->FindNearestCreature(NPC_INCUBATER, 20.0f, true))
                {
                    float x, y;
                    GetPosInRadiusWithRandomOrientation(me, 2.0f, x, y);
                    incubate->CastSpell(x, y, incubate->GetPositionZ(), SPELL_FEATHER_AT, true);
                }
            }
            else               //incubate complete
            {
                if (Creature* incubate = me->FindNearestCreature(NPC_INCUBATER, 20.0f, true))
                {
                    uint8 _data = incubate->GetPositionZ() >= 69.0f ? 1 : 0;
                    if (Creature* juvenile = incubate->SummonCreature(NPC_JUVENILE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 5.0f))
                        juvenile->AI()->SetData(DATA_TAKEOFF, _data);
                }
            }
        }

        void EnterCombat(Unit* who){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mature_egg_of_jikunAI(creature);
    }
};

//69836, 70095
class npc_juvenile : public CreatureScript
{
public:
    npc_juvenile() : CreatureScript("npc_juvenile") { }

    struct npc_juvenileAI : public ScriptedAI
    {
        npc_juvenileAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->SetReactState(REACT_PASSIVE);
            instance = pCreature->GetInstanceScript();
        }
        InstanceScript* instance;
        EventMap events;

        void SetData(uint32 type, uint32 data)
        {
            if (type == DATA_TAKEOFF)
            {
                switch (data)
                {
                //Upper nest front
                case 0:
                    events.RescheduleEvent(EVENT_TAKEOFF, 500);
                    break;
                //Upper nest middle
                case 1:
                    events.RescheduleEvent(EVENT_TAKEOFF_2, 500);
                    break;
                //Lowest nest front
                case 2:
                    events.RescheduleEvent(EVENT_TAKEOFF_3, 500);
                    break;
                }
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == EFFECT_MOTION_TYPE)
            {
                switch (pointId)
                {
                case 3:
                    events.RescheduleEvent(EVENT_ENTERCOMBAT, 500);
                    break;
                case 4:
                    events.RescheduleEvent(EVENT_MOVE_TO_DEST_POS, 500);
                    break;
                case 5:
                    events.RescheduleEvent(EVENT_ENTERCOMBAT, 500);
                    break;
                case 6:
                    events.RescheduleEvent(EVENT_MOVE_TO_DEST_POS_2, 500);
                    break;
                case 7:
                    events.RescheduleEvent(EVENT_ENTERCOMBAT, 500);
                    break;
                }
            }
        }

        float GetRandomAngle()
        {
            float mod = urand(0, 2);
            float mod2 = urand(0, 8);
            return mod + (mod2 / 10);
        }

        void JustDied(Unit* killer)
        {
            me->GetMotionMaster()->MoveFall();
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                //Fly mechanic: juvenile - upper front nest
                case EVENT_TAKEOFF:
                    if (Creature* jikun = me->GetCreature(*me, instance->GetGuidData(NPC_JI_KUN)))
                    {
                        float ang = me->GetAngle(jikun);
                        float dist = me->GetExactDist2d(jikun);
                        float destdist = dist - 15.0f;
                        float x, y;
                        GetPositionWithDistInOrientation(me, destdist, ang, x, y);
                        me->GetMotionMaster()->MoveJump(x, y, jikun->GetPositionZ() + 50.0f, 10.0f, 10.0f, 3);
                    }
                    break;
                case EVENT_ENTERCOMBAT:
                    me->GetMotionMaster()->MoveIdle();
                    me->SetHomePosition(me->GetPosition());
                    DoZoneInCombat(me, 200.0f);
                    events.RescheduleEvent(EVENT_TO_PATROL, 500);
                    break;
                case EVENT_TO_PATROL:
                    me->GetMotionMaster()->MoveRandom(5.0f);
                    events.RescheduleEvent(EVENT_CHEEP, 5000);
                    break;
                //Fly mechanic: juvenile - upper middle nest
                case EVENT_TAKEOFF_2:
                {
                    float _ang = GetRandomAngle();
                    me->SetFacingTo(_ang);
                    float x, y;
                    GetPositionWithDistInOrientation(me, urand(20, 25), _ang, x, y);
                    me->GetMotionMaster()->MoveJump(x, y, me->GetPositionZ(), 10.0f, 10.0f, 4);
                }
                break;
                case EVENT_MOVE_TO_DEST_POS:
                    if (Creature* jikun = me->GetCreature(*me, instance->GetGuidData(NPC_JI_KUN)))
                        me->GetMotionMaster()->MoveJump(me->GetPositionX(), me->GetPositionY(), jikun->GetPositionZ() + 50.0f, 10.0f, 10.0f, 5);
                    break;
                //Fly mechanic: juvenile - lowest front nest
                case EVENT_TAKEOFF_3:
                    if (Creature* jikun = me->GetCreature(*me, instance->GetGuidData(NPC_JI_KUN)))
                    {
                        me->SetFacingToObject(jikun);
                        me->GetMotionMaster()->MoveJump(me->GetPositionX(), me->GetPositionY(), jikun->GetPositionZ() + 10.0f, 10.0f, 10.0f, 6);
                    }
                    break;
                case EVENT_MOVE_TO_DEST_POS_2:
                    if (Creature* jikun = me->GetCreature(*me, instance->GetGuidData(NPC_JI_KUN)))
                    {
                        float ang = me->GetAngle(jikun);
                        float dist = me->GetExactDist2d(jikun);
                        float destdist = dist - 15.0f;
                        float x, y;
                        GetPositionWithDistInOrientation(me, destdist, ang, x, y);
                        me->GetMotionMaster()->MoveJump(x, y, jikun->GetPositionZ() + 50.0f, 10.0f, 10.0f, 7);
                    }
                    break;
                //Battle Spell
                case EVENT_CHEEP:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                        DoCast(target, SPELL_CHEEP_HIGHT);
                    events.RescheduleEvent(EVENT_CHEEP, 8000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_juvenileAI(pCreature);
    }
};

//69885
class npc_jump_to_boss_platform : public CreatureScript
{
public:
    npc_jump_to_boss_platform() : CreatureScript("npc_jump_to_boss_platform") {}

    struct npc_jump_to_boss_platformAI : public ScriptedAI
    {
        npc_jump_to_boss_platformAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE);
        }
        InstanceScript* pInstance;

        void Reset()
        {
            DoCast(me, SPELL_JUMP_TO_B_PLATFORM, true);
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void EnterCombat(Unit* who){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_jump_to_boss_platformAI(creature);
    }
};

class npc_fall_catcher : public CreatureScript
{
public:
    npc_fall_catcher() : CreatureScript("npc_fall_catcher") {}

    struct npc_fall_catcherAI : public ScriptedAI
    {
        npc_fall_catcherAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE);
        }
        InstanceScript* pInstance;
        EventMap events;

        void Reset(){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            switch (pointId)
            {
            case 1:
                if (Creature* jikun = me->GetCreature(*me, pInstance->GetGuidData(NPC_JI_KUN)))
                {
                    me->SetFacingToObject(jikun);
                    events.RescheduleEvent(EVENT_FALLCATCHER_IN_POS, 500);
                }
                break;
            case 2:
                if (Vehicle* fallcatcher = me->GetVehicleKit())
                {
                    if (Unit* target = fallcatcher->GetPassenger(0))
                    {
                        target->ExitVehicle();
                        target->SetCanFly(false);
                        target->RemoveAurasDueToSpell(SPELL_PARACHUTE_BUFF);
                        target->RemoveAurasDueToSpell(SPELL_PARACHUTE);
                        target->RemoveAurasDueToSpell(SPELL_SAFE_NET_TRIGGER);
                        target->RemoveAurasDueToSpell(SPELL_GENTLE_YET_FIRM);
                        target->RemoveAurasDueToSpell(SPELL_CAST_FILTER);
                        target->RemoveFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY);
                        me->DespawnOrUnsummon();
                    }
                }
                break;
            }
        }


        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            me->GetMotionMaster()->MoveCharge(me->GetPositionX(), me->GetPositionY(), -28.29f, 20.0f, 1);
        }

        void EnterCombat(Unit* who){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_FALLCATCHER_IN_POS)
                {
                    if (Creature* jikun = me->GetCreature(*me, pInstance->GetGuidData(NPC_JI_KUN)))
                    {
                        float ang = me->GetAngle(jikun);
                        float dist = me->GetExactDist2d(jikun);
                        float destdist = dist - 35.0f;
                        float x, y;
                        GetPositionWithDistInOrientation(me, destdist, ang, x, y);
                        me->GetMotionMaster()->MoveJump(x, y, -30.86f, 20.0f, 20.0f, 2);
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fall_catcherAI(creature);
    }
};

//68178
class npc_jikun_feed : public CreatureScript
{
public:
    npc_jikun_feed() : CreatureScript("npc_jikun_feed") {}

    struct npc_jikun_feedAI : public ScriptedAI
    {
        npc_jikun_feedAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }
        InstanceScript* pInstance;

        void Reset(){}

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (pointId == SPELL_JUMPS_DOWN_TO_HATCHLING)
                me->DespawnOrUnsummon();
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void EnterCombat(Unit* who){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_jikun_feedAI(creature);
    }
};

//68188
class npc_jikun_feed_pool : public CreatureScript
{
public:
    npc_jikun_feed_pool() : CreatureScript("npc_jikun_feed_pool") {}

    struct npc_jikun_feed_poolAI : public ScriptedAI
    {
        npc_jikun_feed_poolAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE);
        }
        InstanceScript* pInstance;
        EventMap events;

        void Reset()
        {
            DoCast(me, SPELL_FEED_POOL_SPAWN_P, true);
            events.RescheduleEvent(EVENT_FIND_PLAYER, 1000);
        }

        void IsSummonedBy(Unit* summoner)
        {
            if (summoner->ToCreature())
                summoner->ToCreature()->DespawnOrUnsummon();
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void EnterCombat(Unit* who){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_FIND_PLAYER)
                {
                    std::list<Player*> pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 3.0f);
                    if (!pllist.empty())
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                            if (!(*itr)->HasAura(SPELL_FEED_POOL_DMG) && me->GetExactDist2d(*itr) <= 5.5f)
                                me->CastSpell(*itr, SPELL_FEED_POOL_DMG, true);
                    events.RescheduleEvent(EVENT_FIND_PLAYER, 1000);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_jikun_feed_poolAI(creature);
    }
};

//218543
class go_ji_kun_feather : public GameObjectScript
{
public:
    go_ji_kun_feather() : GameObjectScript("go_ji_kun_feather") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        InstanceScript* pInstance = (InstanceScript*)go->GetInstanceScript();
        if (!pInstance)
            return false;

        if (!player->HasAura(SPELL_JIKUN_FLY))
            player->CastSpell(player, SPELL_JIKUN_FLY, true);

        return true;
    }
};

//133755
class spell_jikun_fly : public SpellScriptLoader
{
public:
    spell_jikun_fly() : SpellScriptLoader("spell_jikun_fly") { }

    class spell_jikun_fly_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_jikun_fly_SpellScript);

        SpellCastResult CheckCast()
        {
            if (GetCaster())
            {
                if (GetCaster()->GetMap() && GetCaster()->GetMap()->GetId() == 1098) //Throne of Thunder
                    if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                        if (instance->GetBossState(DATA_JI_KUN) == IN_PROGRESS)
                            return SPELL_CAST_OK;

                //Check cast failed, remove aura
                GetCaster()->RemoveAurasDueToSpell(SPELL_DAEDALIAN_WINGS);
            }
            return SPELL_FAILED_NOT_HERE;
        }

        void HandleAfterCast()
        {
            if (GetCaster())
            {
                if (Aura* aura = GetCaster()->GetAura(SPELL_DAEDALIAN_WINGS))
                {
                    if (aura->GetStackAmount() > 1)
                        aura->SetStackAmount(aura->GetStackAmount() - 1);
                    else
                        aura->Remove();
                }
            }
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_jikun_fly_SpellScript::CheckCast);
            AfterCast += SpellCastFn(spell_jikun_fly_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_jikun_fly_SpellScript();
    }
};

//134339
class spell_daedalian_wings : public SpellScriptLoader
{
public:
    spell_daedalian_wings() : SpellScriptLoader("spell_daedalian_wings") { }

    class spell_daedalian_wings_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_daedalian_wings_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget())
                if (Aura* aura = GetTarget()->GetAura(SPELL_DAEDALIAN_WINGS))
                    if (aura->GetStackAmount() < 4)
                        aura->SetStackAmount(4);
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_daedalian_wings_AuraScript::OnApply, EFFECT_0, SPELL_AURA_OVERRIDE_SPELLS, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_daedalian_wings_AuraScript();
    }
};

//134347, 134335
class spell_incubated : public SpellScriptLoader
{
public:
    spell_incubated() : SpellScriptLoader("spell_incubated") { }

    class spell_incubated_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_incubated_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTarget() && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                if (GetTarget()->ToCreature())
                    GetTarget()->Kill(GetTarget());
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_incubated_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_incubated_AuraScript();
    }
};

class TankFilter
{
public:
    bool operator()(WorldObject* unit)
    {
        if (Player* player = unit->ToPlayer())
            if (!player->isInTankSpec())
                return false;
        return true;
    }
};

//138923
class spell_caw_searcher : public SpellScriptLoader
{
public:
    spell_caw_searcher() : SpellScriptLoader("spell_caw_searcher") { }

    class spell_caw_searcher_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_caw_searcher_SpellScript);

        void FilterTargets(std::list<WorldObject*> &targets)
        {
            if (GetCaster())
            {
                targets.remove_if(TankFilter());
                uint8 targetcount = GetCaster()->GetMap()->Is25ManRaid() ? 5 : 2;
                if (targets.size() > targetcount)
                    targets.resize(targetcount);
            }
        }

        void HandleScript(SpellEffIndex effIndex)
        {
            if (GetCaster() && GetHitUnit())
                GetCaster()->CastSpell(GetHitUnit(), SPELL_CAW_DMG, true);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_caw_searcher_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_caw_searcher_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_caw_searcher_SpellScript();
    }
};

//138319
class spell_jikun_feed_platform_p_dmg : public SpellScriptLoader
{
public:
    spell_jikun_feed_platform_p_dmg() : SpellScriptLoader("spell_jikun_feed_platform_p_dmg") { }

    class spell_jikun_feed_platform_p_dmg_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_jikun_feed_platform_p_dmg_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster() && GetCaster()->ToCreature() && GetTarget())
            {
                if (aurEff->GetTickNumber() > 3)
                {
                    GetCaster()->ToCreature()->DespawnOrUnsummon();
                    GetTarget()->RemoveAurasDueToSpell(SPELL_FEED_POOL_DMG);
                    GetTarget()->CastSpell(GetTarget(), SPELL_SLIMED_DMG, true);
                    return;
                }
                if (GetTarget()->GetExactDist2d(GetCaster()) > 6.0f)
                    GetTarget()->RemoveAurasDueToSpell(SPELL_FEED_POOL_DMG);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_jikun_feed_platform_p_dmg_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_jikun_feed_platform_p_dmg_AuraScript();
    }
};

//134256
class spell_jikun_slimed_aura : public SpellScriptLoader
{
public:
    spell_jikun_slimed_aura() : SpellScriptLoader("spell_jikun_slimed_aura") { }

    class spell_jikun_slimed_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_jikun_slimed_aura_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget() && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                GetTarget()->CastSpell(GetTarget(), SPELL_SLIMED_DEBUFF, true);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_jikun_slimed_aura_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_jikun_slimed_aura_AuraScript();
    }
};

//140741
class spell_primal_nutriment : public SpellScriptLoader
{
public:
    spell_primal_nutriment() : SpellScriptLoader("spell_primal_nutriment") { }

    class spell_primal_nutriment_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_primal_nutriment_SpellScript);

        SpellCastResult CheckCast()
        {
            if (GetCaster() && GetCaster()->ToCreature() && GetExplTargetUnit())
            {
                if (!GetExplTargetUnit()->HasAura(SPELL_FLY))
                    return SPELL_FAILED_UNKNOWN;

                if (!GetExplTargetUnit()->HasUnitMovementFlag(MOVEMENTFLAG_FLYING))
                    return SPELL_FAILED_UNKNOWN;

                GetCaster()->ToCreature()->DespawnOrUnsummon();
                return SPELL_CAST_OK;
            }
            return SPELL_FAILED_UNKNOWN;
        }

        void Register()
        {
            OnCheckCast += SpellCheckCastFn(spell_primal_nutriment_SpellScript::CheckCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_primal_nutriment_SpellScript();
    }
};

//134321
class spell_jikun_hatchling_eat : public SpellScriptLoader
{
public:
    spell_jikun_hatchling_eat() : SpellScriptLoader("spell_jikun_hatchling_eat") { }

    class spell_jikun_hatchling_eat_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_jikun_hatchling_eat_AuraScript);

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE && GetCaster() && GetTarget())
            {
                if (GetTarget()->ToCreature() && GetCaster()->ToCreature())
                {
                    GetTarget()->ToCreature()->DespawnOrUnsummon();
                    GetCaster()->ToCreature()->AI()->SetData(DATA_MORPH, 0);
                }
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_jikun_hatchling_eat_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_jikun_hatchling_eat_AuraScript();
    }
};

//137528
class spell_jikun_feed_young : public SpellScriptLoader
{
public:
    spell_jikun_feed_young() : SpellScriptLoader("spell_jikun_feed_young") { }

    class spell_jikun_feed_young_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_jikun_feed_young_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                {
                    uint32 tick = aurEff->GetTickNumber();
                    switch (tick)
                    {
                    case 1:
                        instance->SetData(DATA_LAUNCH_FEED_NEST, 0);
                        break;
                    case 2:
                        instance->SetData(DATA_LAUNCH_FEED_PLATFORM, 0);
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_jikun_feed_young_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_jikun_feed_young_AuraScript();
    }
};

//8848
class at_jikun_precipice : public AreaTriggerScript
{
public:
    at_jikun_precipice() : AreaTriggerScript("at_jikun_precipice") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* areaTrigger, bool enter)
    {
        if (enter)
        {
            player->RemoveUnitMovementFlag(MOVEMENTFLAG_FALLING);
            player->SetCanFly(true);
            player->StopMoving();
            player->GetMotionMaster()->Clear(true);
            player->RemoveAurasDueToSpell(SPELL_PARACHUTE_BUFF);
            player->RemoveAurasDueToSpell(SPELL_PARACHUTE);
            player->CastSpell(player, SPELL_SAFE_NET_TRIGGER, true);
            player->SetFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_ALLOW_ONLY_ABILITY); //for safe(block all ability)
        }
        return true;
    }
};

void AddSC_boss_jikun()
{
    new boss_jikun();
    new npc_incubater();
    new npc_young_egg_of_jikun();
    new npc_hatchling();
    new npc_jikun_feed_pool_nest();
    new npc_jikun_fledgling_egg();
    new npc_mature_egg_of_jikun();
    new npc_juvenile();
    new npc_jump_to_boss_platform();
    new npc_fall_catcher();
    new npc_jikun_feed();
    new npc_jikun_feed_pool();
    new go_ji_kun_feather();
    new spell_jikun_fly();
    new spell_daedalian_wings();
    new spell_incubated();
    new spell_caw_searcher();
    new spell_jikun_feed_platform_p_dmg();
    new spell_jikun_slimed_aura();
    new spell_primal_nutriment();
    new spell_jikun_hatchling_eat();
    new spell_jikun_feed_young();
    new at_jikun_precipice();
}
