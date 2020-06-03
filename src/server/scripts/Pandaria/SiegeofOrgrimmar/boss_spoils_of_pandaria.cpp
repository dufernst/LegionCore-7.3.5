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
    //Big 
    SPELL_STRENGTH_OF_THE_STONE       = 145998,
    SPELL_SHADOW_VOLLEY               = 148515,
    SPELL_SHADOW_VOLLEY_D             = 148516,
    SPELL_MOLTEN_FIST                 = 148518,
    SPELL_MOLTEN_FIST_D               = 148517,
    SPELL_JADE_TEMPEST                = 148582,
    SPELL_JADE_TEMPEST_D              = 148583,
    SPELL_FRACTURE                    = 148513,
    SPELL_FRACTURE_D                  = 148514,
    SPELL_SET_TO_BLOW_DMG             = 145993,
    SPELL_SET_TO_BLOW_AURA            = 145987,
    SPELL_SET_TO_BLOW_AT              = 146365,
    SPELL_SET_TO_BLOW_VISUAL_1        = 142997,
    SPELL_SET_TO_BLOW_VISUAL_2        = 148054,
    SPELL_SET_TO_BLOW_VISUAL_3        = 148055,
    SPELL_SET_TO_BLOW_VISUAL_4        = 148056,
    SPELL_PHEROMONE_CLOUD             = 148762,
    SPELL_PHEROMONE_CLOUD_DMG         = 148760,

    //Medium
    SPELL_CRIMSON_RECOSTITUTION_AT    = 145270,
    SPELL_FORBIDDEN_MAGIC             = 145230,
    SPELL_MOGU_RUNE_OF_POWER_AT       = 145460,
    SPELL_RESIDUE                     = 145786,
    SPELL_RAGE_OF_THE_EMPRESS         = 145812,
    SPELL_RAGE_OF_THE_EMPRESS_AURA    = 145813,
    SPELL_WINDSTORM_AT                = 145286,
    SPELL_MATTER_SCRAMBLE             = 145288,
    SPELL_TORMENT_DUMMY               = 142942,
    SPELL_TORMENT_DMG                 = 136885,

    //Small
    SPELL_EARTHEN_SHARD               = 144923,
    SPELL_HARDEN_FLESH_DMG            = 145218,   
    SPELL_RUSH                        = 144904,
    SPELL_KW_ENRAGE                   = 145692,
    SPELL_MANTID_SWARM                = 145806,
    SPELL_MANTID_SWARM2               = 145807,
    SPELL_MANTID_SWARM3               = 145808,
    SPELL_THROW_EXPLOSIVES            = 145702,
    SPELL_GUSTING_BOMB                = 145712,
    SPELL_GUSTING_BOMB_AOE_DMG        = 145718,
    SPELL_GUSTING_BOMB_AT             = 145714,
    SPELL_ENCAPSULATED_PHEROMONES     = 142524,
    SPELL_ENCAPSULATED_VISUAL         = 142492,
    SPELL_ENCAPSULATED_PHEROMONES_AT  = 145285,
    //burial urn
    SPELL_SPARK_OF_LIFE_CREATER       = 142694,
    //spark of life
    SPELL_SPARK_OF_LIFE_VISUAL        = 142676,
    SPELL_PULSE                       = 142765,
    SPELL_NOVA                        = 142775,

    //Pandaren Relic Box
    SPELL_EMINENCE                    = 146189,
    SPELL_GUSTING_CRANE_KICK          = 146180,
    SPELL_GUSTING_CRANE_KICK_DMG      = 146182,
    SPELL_KEG_TOSS                    = 146214,
    SPELL_KEG_TOSS_DMG                = 146217,
    SPELL_BREATH_OF_FIRE              = 146222,
    SPELL_BREATH_OF_FIRE_DMG          = 146226,
    SPELL_BREATH_OF_FIRE_TR           = 146235,
    SPELL_PATH_OF_BLOSSOMS_AT         = 146255,
    SPELL_PATH_OF_BLOSSOMS_J          = 146256,
    SPELL_PATH_OF_BLOSSOMS_DMG        = 146257,
    SPELL_MASS_PARALYSIS              = 146289,

    //Special
    SPELL_GHOST_VISUAL                = 118205,
    SPELL_LIFT_HOOK_VISUAL            = 142721,
    SPELL_AURA_BAR                    = 144921,
    SPELL_AURA_BAR_S                  = 148505,
    SPELL_SOOPS_AT_VISUAL             = 145687,
    SPELL_UNSTABLE_DEFENSE_SYSTEMS    = 145685, //Periodic Dummy
    SPELL_UNSTABLE_DEFENSE_SYSTEMS_D  = 145688, //Dmg
    SPELL_BLADE_OF_THE_HUNDRED_STEPS  = 146068, //tank 
    SPELL_STAFF_OF_RESONATING_WATER   = 146099, //healer
    SPELL_CLAW_OF_BURNING_ANGER       = 146141, //dd
    //HM
    SPELL_UNSTABLE_SPARK_DUMMY_SPAWN  = 146824,
    SPELL_UNSTABLE_SPARK_SPAWN_VISUAL = 146696,
    SPELL_SUPERNOVA                   = 146815,

    SPELL_STONE_STATUE_SUMMON         = 145489,
    SPELL_ANIMATED_STRIKE_SIT         = 145499,
    SPELL_ANIMATED_STRIKE_LAUNCH_V    = 142944,
    SPELL_ANIMATED_STRIKE_DMG         = 145523,
};

enum Events
{
    //Lift hook
    EVENT_START_LIFT                  = 1,
    EVENT_POINT                       = 2,
    EVENT_BACK                        = 3,
    EVENT_RESET                       = 4,

    //Summons
    EVENT_EARTHEN_SHARD               = 5, 
    EVENT_SHADOW_VOLLEY               = 6,
    EVENT_MOLTEN_FIST                 = 7,
    EVENT_JADE_TEMPEST                = 8,
    EVENT_FRACTURE                    = 9,
    EVENT_HARDEN_FLESH                = 10,
    EVENT_FIND_PLAYERS                = 11,
    EVENT_RUSH                        = 12,
    EVENT_KW_ENRAGE                   = 13,
    EVENT_IN_PROGRESS                 = 14,
    EVENT_KEG_TOSS                    = 15,
    EVENT_BREATH_OF_FIRE              = 16,
    EVENT_PATH_OF_BLOSSOMS            = 17,
    EVENT_PATH_OF_BLOSSOMS_PROGRESS   = 18,
    EVENT_MANTID_SWARM                = 19,
    EVENT_THROW_EXPLOSIVES            = 20,
    EVENT_FORBIDDEN_MAGIC             = 21,
    EVENT_MOGU_RUNE_OF_POWER          = 22,
    EVENT_GUSTING_CRANE_KICK          = 23,
    EVENT_CRIMSON_RECOSTITUTION       = 24,
    EVENT_SET_TO_BLOW                 = 25,
    EVENT_RESIDUE                     = 26,
    EVENT_RAGE_OF_THE_EMPRESS         = 27,
    EVENT_WINDSTORM                   = 28,
    EVENT_GUSTING_BOMB                = 29,
    EVENT_MATTER_SCRAMBLE             = 30,
    EVENT_TORMENT                     = 31,
    EVENT_START                       = 32,
    EVENT_START_2                     = 33,
    EVENT_START_3                     = 34,
    EVENT_START_SOP                   = 35,
    EVENT_OUTRO                       = 36,
    EVENT_ACTIVE                      = 37,
    EVENT_ANIMATED_STRIKE_START       = 38,
    EVENT_ANIMATED_STRIKE_LAUNCH      = 39,
    EVENT_ANIMATED_STRIKE_FINISH      = 40,
    EVENT_ANIMATED_STRIKE_RESET       = 41,
    EVENT_SUMMON_STONE_STATUE         = 42,
    EVENT_MOVE_STATUE                 = 43,
};

enum Says
{
    SAY_START                         = 0,
    SAY_START_2                       = 1,
    SAY_START_3                       = 2,
    SAY_START_ENCOUNTER               = 3,
    SAY_FIRST_MODUL_PRE               = 4,
    SAY_FIRST_MODUL_DONE              = 5,
    SAY_SECOND_MODUL_PRE              = 6,
    SAY_SECOND_MODUL_DONE             = 7,
    SAY_SYSTEM_REBOOT                 = 8,
};

Position dpos[4] =
{
    { 1606.77f, -5124.41f, -263.3986f, 0.0f },
    { 1657.45f, -5127.57f, -263.3986f, 0.0f },
    { 1639.86f, -5101.63f, -263.3986f, 0.0f },
    { 1622.80f, -5148.56f, -263.3986f, 0.0f },
};

//Big
uint32 bigmoguentry[4] =
{
    NPC_JUN_WEI,
    NPC_ZU_YIN,
    NPC_XIANG_LIN,
    NPC_KUN_DA,
};

uint32 bigmantisentry[4] =
{
    NPC_COMMANDER_ZAKTAR,
    NPC_COMMANDER_IKTAL,
    NPC_COMMANDER_NAKAZ,
    NPC_COMMANDER_TIK,
};
//

//Mediom
uint32 mediummoguentry[2] =
{
    NPC_MODIFIED_ANIMA_GOLEM,
    NPC_MOGU_SHADOW_RITUALIST,
};

uint32 mediummantisentry[2] =
{
    NPC_ZARTHIK_AMBER_PRIEST,
    NPC_SETTHIK_WIND_WIELDER,
};
//

//Small
uint32 smallmoguentry[3] =
{
    NPC_QUILEN_GUARDIANS,
    NPC_ANIMATED_STONE_MOGU,
    NPC_BURIAL_URN,
};

uint32 smallmantisentry[3] =
{
    NPC_SRITHIK_BOMBARDIER,
    NPC_AMBER_ENCASED_KUNCHONG, //not found visual spell for Encapsulated Pheromones
    NPC_KORTHIK_WARCALLER,
};
//

uint32 mantidswarm[3] =
{
    SPELL_MANTID_SWARM,
    SPELL_MANTID_SWARM2,
    SPELL_MANTID_SWARM3,
};

//Pandaren Relic box
uint32 pandarenrelicentry[3] = 
{
    NPC_NAMELESS_WINDWALKER_SPIRIT,
    NPC_WISE_MISTWEAVER_SPIRIT,
    NPC_ANCIENT_BREWMASTER_SPIRIT,
};

enum sActions
{
    ACTION_SEND_AURA_BAR         = 1,
    ACTION_SECOND_ROOM           = 2,
    ACTION_RE_ATTACK             = 3,
};

enum sData
{
    DATA_BUFF_HEALER             = 1,
    DATA_BUFF_DD                 = 2,
    DATA_BUFF_TANK               = 3,
    DATA_UPDATE_POWER            = 4,
    DATA_GET_SPIRIT_SUM_COUNT    = 5,
    DATA_ONE_MODUL_READY         = 6,
};

uint32 spellbuff[3] =
{
    SPELL_STAFF_OF_RESONATING_WATER,
    SPELL_CLAW_OF_BURNING_ANGER,
    SPELL_BLADE_OF_THE_HUNDRED_STEPS,   
};

uint32 spoilsentry[4] =
{
    NPC_MOGU_SPOILS,
    NPC_MOGU_SPOILS2,
    NPC_MANTIS_SPOILS,
    NPC_MANTIS_SPOILS2,
};

#define GOSSIP1 "We are ready"

//71889
class npc_ssop_spoils : public CreatureScript
{
public:
    npc_ssop_spoils() : CreatureScript("npc_ssop_spoils") {}

    struct npc_ssop_spoilsAI : public ScriptedAI
    {
        npc_ssop_spoilsAI(Creature* creature) : ScriptedAI(creature), _summons(me)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(11686);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            lastcount = 270;
            newcount = 0;
            firstlevel = false;
            secondlever = false;
            done = false;
            lfl.clear();
        }
        
        InstanceScript* instance;
        SummonList _summons;
        EventMap events;
        std::list<ObjectGuid>lfl;
        uint32 lastcount;
        uint32 newcount;
        bool firstlevel, secondlever, done;

        void Reset(){}

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void JustSummoned(Creature* sum)
        {
            _summons.Summon(sum);
        }

        void SetData(uint32 type, uint32 data)
        {
            if (type == DATA_ONE_MODUL_READY)
            {
                switch (data)
                {
                case 0:
                    if (!firstlevel)
                    {
                        firstlevel = true;
                        ZoneTalk(SAY_FIRST_MODUL_PRE);
                    }
                    break;
                case 1:
                    if (!secondlever)
                    {
                        secondlever = true;
                        ZoneTalk(SAY_SECOND_MODUL_PRE);
                    }
                    break;
                default:
                    break;
                }
            }
        }

        void GenerateListForLoot()
        {
            std::list<Player*>pllist;
            pllist.clear();
            GetPlayerListInGrid(pllist, me, 100.0f);
            if (!pllist.empty())
                for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); ++itr)
                    lfl.push_back((*itr)->GetGUID());
        }
        
        void DoAction(int32 const action)
        {
            if (instance)
            {
                switch (action)
                {
                case ACTION_SSOPS_IN_PROGRESS:
                    ZoneTalk(SAY_START);
                    GenerateListForLoot();
                    events.RescheduleEvent(EVENT_START_2, 9000);
                    break;
                case ACTION_SSOPS_SECOND_ROOM:
                    events.Reset();
                    ZoneTalk(SAY_FIRST_MODUL_DONE);
                    lastcount = 300;
                    events.RescheduleEvent(EVENT_IN_PROGRESS, 1000);
                    break;
                case ACTION_SSOPS_DONE:
                    if (!done)
                    {
                        done = true;
                        me->setFaction(35);
                        ZoneTalk(SAY_SECOND_MODUL_DONE);
                        events.Reset();
                        _summons.DespawnAll();
                        DespawnAllAT();
                        OfflineWorldState();
                        if (!me->GetMap()->IsLfr())
                            if (GameObject* chest = me->SummonGameObject(GO_NSOP_SPOILS, 1631.8f, -5125.97f, -271.122f, 5.31506f, 0.0f, 0.0f, 0.0f, 1.0f, 604800))
                                chest->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);

                        Map::PlayerList const &PlayerList = me->GetMap()->GetPlayers();
                        if (!PlayerList.isEmpty())
                        {
                            for (Map::PlayerList::const_iterator Itr = PlayerList.begin(); Itr != PlayerList.end(); ++Itr)
                            {
                                if (Player* player = Itr->getSource())
                                {
                                    player->UpdateAchievementCriteria(CRITERIA_TYPE_BE_SPELL_TARGET, 145904);
                                    if (me->GetMap()->IsHeroic())
                                        if (AchievementEntry const* achievementEntry = sAchievementStore.LookupEntry(8478))
                                            if (!player->HasAchieved(8478))
                                                player->CompletedAchievement(achievementEntry);
                                }
                            }
                        }
                        lfl.clear();
                        me->GeneratePersonalLoot(me, NULL); // bonus loot
                        events.RescheduleEvent(EVENT_OUTRO, 4000);
                    }
                    break;
                }
            }
        }

        void DespawnAllAT()
        {
            std::list<AreaTrigger*> atlist;
            atlist.clear();
            me->GetAreaTriggersWithEntryInRange(atlist, 5269, me->GetGUID(), 50.0f);
            me->GetAreaTriggersWithEntryInRange(atlist, 5299, me->GetGUID(), 100.0f);
            if (!atlist.empty())
                for (std::list<AreaTrigger*>::const_iterator itr = atlist.begin(); itr != atlist.end(); itr++)
                    (*itr)->RemoveFromWorld();
        }

        void OfflineWorldState()
        {
            Map::PlayerList const &players = me->GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                if (Player* pl = i->getSource())
                    if (pl->isAlive())
                        pl->SendUpdateWorldState(8431, 0);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_UNSTABLE_DEFENSE_SYSTEMS);
        }

        void SendWipe()
        {
            if (instance)
            {
                events.Reset();
                Map::PlayerList const &players = me->GetMap()->GetPlayers();
                for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                    if (Player* pl = i->getSource())
                        if (pl->isAlive())
                            pl->Kill(pl, true);

                DespawnAllAT();
                OfflineWorldState();
                _summons.DespawnAll();
                instance->SetBossState(DATA_SPOILS_OF_PANDARIA, NOT_STARTED);
                firstlevel = false;
                secondlever = false;
                done = false;
                lastcount = 270;
                newcount = 0;
                lfl.clear();
            }
        }
        
        void UpdateAI(uint32 diff)
        {
            if (!instance)
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_START_2:
                    ZoneTalk(SAY_START_2);
                    events.RescheduleEvent(EVENT_START_3, 5000);
                    break;
                case EVENT_START_3:
                    ZoneTalk(SAY_START_3);
                    events.RescheduleEvent(EVENT_START_SOP, 7000);
                    break;
                case EVENT_START_SOP:
                    instance->SetData(DATA_SOP_START, 0);
                    ZoneTalk(SAY_START_ENCOUNTER);
                    DoCast(me, SPELL_SOOPS_AT_VISUAL, true);
                    events.RescheduleEvent(EVENT_IN_PROGRESS, 1000);
                    break;
                case EVENT_OUTRO:
                    ZoneTalk(SAY_SYSTEM_REBOOT);
                    break;
                case EVENT_IN_PROGRESS:
                    if (instance->IsWipe())
                    {
                        events.Reset();
                        DespawnAllAT();
                        OfflineWorldState();
                        _summons.DespawnAll();
                        firstlevel = false;
                        secondlever = false;
                        done = false;
                        lastcount = 270;
                        newcount = 0;
                        lfl.clear();
                        instance->SetBossState(DATA_SPOILS_OF_PANDARIA, NOT_STARTED);
                    }
                    else
                    {
                        newcount = lastcount ? lastcount - 1 : 0;
                        Map::PlayerList const &players = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                        {
                            if (Player* pl = i->getSource())
                            {
                                pl->SendUpdateWorldState(8431, 1);
                                pl->SendUpdateWorldState(8381, newcount);
                            }
                        }
                        lastcount = newcount;

                        if (!newcount)
                            SendWipe();
                        else
                            events.RescheduleEvent(EVENT_IN_PROGRESS, 1000);
                    }
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ssop_spoilsAI(creature);
    }
};

//73720, 73722, 71512, 73721
class npc_generic_spoil : public CreatureScript
{
public:
    npc_generic_spoil() : CreatureScript("npc_generic_spoil") {}

    struct npc_generic_spoilAI : public ScriptedAI
    {
        npc_generic_spoilAI(Creature* creature) : ScriptedAI(creature), summons(me)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }

        InstanceScript* instance;
        SummonList summons;
        EventMap events;
        ObjectGuid mspoilGuid;      //main spoil 
        ObjectGuid othermspoilguid; //other main spoil 
        uint8 power;
        uint8 newssc;               //
        uint8 lastssc;              //spirit summon count 

        void Reset()
        {
            newssc = 0;
            lastssc = 0; 
            power = 0;
            mspoilGuid.Clear();
        }

        void EnterCombat(Unit* who){}

        void JustSummoned(Creature* sum)
        {
            summons.Summon(sum);
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
                case DATA_BUFF_DD:
                case DATA_BUFF_TANK:
                case DATA_BUFF_HEALER:
                {
                    std::list<Player*> pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 50.0f);
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                            if ((*itr)->GetSpecializationRole() == type)
                                (*itr)->CastSpell(*itr, spellbuff[type-1], true);
                    }
                    break;
                }
                case DATA_UPDATE_POWER:
                    power = power + data > 50 ? 50 : power + data;
                    break;
                default:
                    break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_GET_SPIRIT_SUM_COUNT)
            {
                const_cast<npc_generic_spoilAI*>(this)->CalcVariable();
                return (uint32(newssc));
            }
            return 0;
        }

        void CalcVariable()
        {
            if (!lastssc)
                newssc = urand(0, 2);
            else if (lastssc >= 2)
                newssc = 0;
            else
                newssc++;
            lastssc = newssc;
        }

        void ActivateOrOfflineBoxes(bool state)
        {
            std::list<GameObject*> boxlist;
            boxlist.clear();
            switch (me->GetEntry())
            {
            case NPC_MOGU_SPOILS:
            case NPC_MOGU_SPOILS2:
                GetGameObjectListWithEntryInGrid(boxlist, me, GO_SMALL_MOGU_BOX, 50.0f);
                GetGameObjectListWithEntryInGrid(boxlist, me, GO_MEDIUM_MOGU_BOX, 50.0f);
                GetGameObjectListWithEntryInGrid(boxlist, me, GO_BIG_MOGU_BOX, 50.0f);
                GetGameObjectListWithEntryInGrid(boxlist, me, GO_PANDAREN_RELIC_BOX, 50.0f);
                if (!boxlist.empty())
                {
                    for (std::list<GameObject*>::const_iterator itr = boxlist.begin(); itr != boxlist.end(); itr++)
                    {
                        if (state)
                            (*itr)->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        else
                            (*itr)->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }

                }
                break;
            case NPC_MANTIS_SPOILS:
            case NPC_MANTIS_SPOILS2:
                GetGameObjectListWithEntryInGrid(boxlist, me, GO_SMALL_MANTIS_BOX, 50.0f);
                GetGameObjectListWithEntryInGrid(boxlist, me, GO_MEDIUM_MANTIS_BOX, 50.0f);
                GetGameObjectListWithEntryInGrid(boxlist, me, GO_BIG_MANTIS_BOX, 50.0f);
                GetGameObjectListWithEntryInGrid(boxlist, me, GO_PANDAREN_RELIC_BOX, 50.0f);
                if (!boxlist.empty())
                {
                    for (std::list<GameObject*>::const_iterator itr = boxlist.begin(); itr != boxlist.end(); itr++)
                    {
                        if (state)
                            (*itr)->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        else
                            (*itr)->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    }
                }
                break;
            }
        }

        void SetGUID(ObjectGuid const& guid, int32 type) override
        {
            if (type == 1)
            {
                mspoilGuid = guid; 
                switch (me->GetEntry())
                {
                case NPC_MOGU_SPOILS:
                case NPC_MOGU_SPOILS2:
                    othermspoilguid = instance->GetGuidData(DATA_SPOIL_MANTIS);
                    break;
                case NPC_MANTIS_SPOILS:
                case NPC_MANTIS_SPOILS2:
                    othermspoilguid = instance->GetGuidData(DATA_SPOIL_MOGU);
                    break;
                }
                ActivateOrOfflineBoxes(true);
                events.RescheduleEvent(EVENT_FIND_PLAYERS, 1000);
            }
        }
        
        void DoAction(int32 const action)
        {
            if (instance)
            {
                switch (action)
                {
                case ACTION_RESET:
                    events.Reset();
                    summons.DespawnAll();
                    power = 0;
                    newssc = 0;
                    lastssc = 0;
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                    me->RemoveAurasDueToSpell(SPELL_AURA_BAR_S);
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_AURA_BAR);
                    break;
                case ACTION_IN_PROGRESS:
                    if (uint32(me->GetPositionZ()) == -271)
                        me->CastSpell(me, SPELL_AURA_BAR_S, true);
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                    if (Creature* spoiltrigger = me->GetCreature(*me, instance->GetGuidData(me->GetEntry())))
                        spoiltrigger->AI()->SetGUID(me->GetGUID(), 1);
                    break;
                case ACTION_SECOND_ROOM:
                    ActivateOrOfflineBoxes(false);
                    if (GameObject* go = me->GetMap()->GetGameObject(instance->GetGuidData(me->GetEntry() == NPC_MOGU_SPOILS2 ? GO_LEVER_R : GO_LEVER_L)))
                        go->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                    break;
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_FIND_PLAYERS:
                    if (!instance)
                        return;

                    std::list<Player*> pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 55.0f);
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                            if (uint32((*itr)->GetPositionZ()) <= -280 && me->GetDistance(*itr) <= 55.0f)
                                if (!(*itr)->HasAura(SPELL_AURA_BAR))
                                    (*itr)->CastCustomSpell(SPELL_AURA_BAR, SPELLVALUE_BASE_POINT0, power, *itr, true);
                                else
                                    (*itr)->SetPower(POWER_ALTERNATE, power, true);

                        //Send new power in frame
                        if (Creature* mspoil = me->GetCreature(*me, mspoilGuid))
                            if (mspoil->HasAura(SPELL_AURA_BAR_S))
                                mspoil->SetPower(POWER_ALTERNATE, power, true);
                    }
                    //Check spoils, if power 50, go second room
                    if (power == 50)
                    {
                        if (Creature* omspoil = me->GetCreature(*me, othermspoilguid))
                        {
                            if (omspoil->HasAura(SPELL_AURA_BAR_S) && omspoil->GetPower(POWER_ALTERNATE) == 50)
                            {
                                if (instance->GetBossState(DATA_SPOILS_OF_PANDARIA) == SPECIAL)
                                {
                                    instance->SetBossState(DATA_SPOILS_OF_PANDARIA, DONE);
                                    break;
                                }
                                else
                                {
                                    DoAction(ACTION_SECOND_ROOM);
                                    break;
                                }
                            }
                            else
                                if (Creature* ssops = me->GetCreature(*me, instance->GetGuidData(NPC_SSOP_SPOILS)))
                                    ssops->AI()->SetData(DATA_ONE_MODUL_READY, (instance->GetBossState(DATA_SPOILS_OF_PANDARIA) == SPECIAL ? 1 : 0));
                        }
                    }
                    events.RescheduleEvent(EVENT_FIND_PLAYERS, 1000);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_generic_spoilAI(creature);
    }
};

//72281
class npc_lever : public CreatureScript
{
public:
    npc_lever() : CreatureScript("npc_lever") {}

    struct npc_leverAI : public ScriptedAI
    {
        npc_leverAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }

        InstanceScript* instance;

        void Reset(){}

        void UpdateAI(uint32 diff){}
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_leverAI(creature);
    }
};

//72972
class npc_lift_hook : public CreatureScript
{
public:
    npc_lift_hook() : CreatureScript("npc_lift_hook") {}

    struct npc_lift_hookAI : public ScriptedAI
    {
        npc_lift_hookAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->AddAura(SPELL_LIFT_HOOK_VISUAL, me);
            count = 4;
        }

        InstanceScript* instance;
        EventMap events;
        uint8 count;

        void Reset(){}

        void EnterCombat(Unit* who){}

        void OnSpellClick(Unit* clicker)
        {
            if (int32(me->GetPositionX()) == 1598)
                count = 0;
            else if (int32(me->GetPositionX()) == 1665)
                count = 1;
            else if (int32(me->GetPositionX()) == 1652)
                count = 2;
            else if (int32(me->GetPositionX()) == 1613)
                count = 3;

            if (count > 3)
                return;

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            clicker->CastSpell(me, 146500, true);
            events.RescheduleEvent(EVENT_START_LIFT, 1000);
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (pointId)
                {
                case 0:
                    if (Vehicle* base = me->GetVehicleKit())
                        if (Unit* passenger = base->GetPassenger(0))
                            passenger->RemoveAurasDueToSpell(SPELL_AURA_BAR);
                    events.RescheduleEvent(EVENT_POINT, 500);
                    break;
                case 1:
                    if (Vehicle* base = me->GetVehicleKit())
                        if (Unit* passenger = base->GetPassenger(0))
                            passenger->ExitVehicle();
                    events.RescheduleEvent(EVENT_BACK, 1000);
                    break;
                case 2:
                    events.RescheduleEvent(EVENT_RESET, 9000);
                    break;
                case 3:
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    break;
                }
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_START_LIFT:
                    me->GetMotionMaster()->MoveCharge(me->GetPositionX(), me->GetPositionY(), -263.3986f, 20.0f, 0);
                    break;
                case EVENT_POINT:      
                    me->GetMotionMaster()->MoveCharge(dpos[count].GetPositionX(), dpos[count].GetPositionY(), -263.3986f, 20.0f, 1);
                    break;
                case EVENT_BACK:
                {
                    Position pos = me->GetHomePosition();
                    me->GetMotionMaster()->MoveCharge(pos.GetPositionX(), pos.GetPositionY(), -263.3986f, 20.0f, 2);
                    break;
                }
                case EVENT_RESET:
                {
                    Position pos = me->GetHomePosition();
                    me->GetMotionMaster()->MoveCharge(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), 20.0f, 3);
                }
                break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lift_hookAI(creature);
    }
};

class npc_generic_sop_units : public CreatureScript
{
public:
    npc_generic_sop_units() : CreatureScript("npc_generic_sop_units") {}

    struct npc_generic_sop_unitsAI : public ScriptedAI
    {
        npc_generic_sop_unitsAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            spawn = 0;
            switch (me->GetEntry())
            {
            case NPC_NAMELESS_WINDWALKER_SPIRIT:
            case NPC_WISE_MISTWEAVER_SPIRIT:
            case NPC_ANCIENT_BREWMASTER_SPIRIT:
                DoCast(me, SPELL_GHOST_VISUAL, true);
                break;
            case NPC_AMBER_ENCASED_KUNCHONG:
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                break;
            default:
                break;
            }
        }
        InstanceScript* instance;
        EventMap events;
        uint32 spawn;

        void Reset()
        {
            switch (me->GetEntry())
            {
            case NPC_JUN_WEI:
            case NPC_ZU_YIN:
            case NPC_XIANG_LIN:
            case NPC_KUN_DA:
                DoCast(me, SPELL_STRENGTH_OF_THE_STONE, true);
                break;
            default:
                break;
            }
            spawn = 1500;
        }

        void EnterCombat(Unit* who)
        {
            switch (me->GetEntry())
            {
            //Big 
            case NPC_JUN_WEI:
                events.RescheduleEvent(EVENT_SHADOW_VOLLEY, 8000);
                events.RescheduleEvent(EVENT_SUMMON_STONE_STATUE, 3000);
                break;
            case NPC_ZU_YIN:
                events.RescheduleEvent(EVENT_MOLTEN_FIST, 8000);
                events.RescheduleEvent(EVENT_SUMMON_STONE_STATUE, 3000);
                break;
            case NPC_XIANG_LIN:
                events.RescheduleEvent(EVENT_JADE_TEMPEST, 8000);
                events.RescheduleEvent(EVENT_SUMMON_STONE_STATUE, 3000);
                break;
            case NPC_KUN_DA:
                events.RescheduleEvent(EVENT_FRACTURE, 8000);
                events.RescheduleEvent(EVENT_SUMMON_STONE_STATUE, 3000);
                break;
            case NPC_COMMANDER_ZAKTAR:
            case NPC_COMMANDER_IKTAL:
            case NPC_COMMANDER_NAKAZ:
            case NPC_COMMANDER_TIK:
                DoCast(me, SPELL_PHEROMONE_CLOUD, true);
                events.RescheduleEvent(EVENT_SET_TO_BLOW, 5000);
                break;
            //Medoum
            case NPC_MOGU_SHADOW_RITUALIST:
                events.RescheduleEvent(EVENT_TORMENT, 7000);
                events.RescheduleEvent(EVENT_FORBIDDEN_MAGIC, 10000);
                events.RescheduleEvent(EVENT_MOGU_RUNE_OF_POWER, 5000);
                break;
            case NPC_MODIFIED_ANIMA_GOLEM:
                events.RescheduleEvent(EVENT_MATTER_SCRAMBLE, 7000);
                events.RescheduleEvent(EVENT_CRIMSON_RECOSTITUTION, 8000);
                break;
            case NPC_ZARTHIK_AMBER_PRIEST:
                events.RescheduleEvent(EVENT_RESIDUE, 5000);
                events.RescheduleEvent(EVENT_MANTID_SWARM, 2000);
                break;
            case NPC_SETTHIK_WIND_WIELDER:
                events.RescheduleEvent(EVENT_RAGE_OF_THE_EMPRESS, 1000);
                events.RescheduleEvent(EVENT_WINDSTORM, 3000);
                break;
            //Small
            case NPC_AMBER_ENCASED_KUNCHONG:
                DoCast(me, SPELL_ENCAPSULATED_VISUAL, true);
                DoCast(me, SPELL_ENCAPSULATED_PHEROMONES, true);
                break;
            case NPC_QUILEN_GUARDIANS:
                events.RescheduleEvent(EVENT_RUSH, 1000);
                break;
            case NPC_ANIMATED_STONE_MOGU:
                events.RescheduleEvent(EVENT_EARTHEN_SHARD, 5000);
                events.RescheduleEvent(EVENT_HARDEN_FLESH, 10000);
                break;
            case NPC_KORTHIK_WARCALLER:
                events.RescheduleEvent(EVENT_KW_ENRAGE, 4000);
                break;
            case NPC_SRITHIK_BOMBARDIER:
                events.RescheduleEvent(EVENT_THROW_EXPLOSIVES, 3000);
                events.RescheduleEvent(EVENT_GUSTING_BOMB, 5000);
                break;
            //Pandaren Relic box
            case NPC_NAMELESS_WINDWALKER_SPIRIT:
                events.RescheduleEvent(EVENT_PATH_OF_BLOSSOMS, 8000);
                break;
            case NPC_ANCIENT_BREWMASTER_SPIRIT:
                events.RescheduleEvent(EVENT_KEG_TOSS, 5000);
                events.RescheduleEvent(EVENT_BREATH_OF_FIRE, 10000);
                break;
            case NPC_WISE_MISTWEAVER_SPIRIT:
                DoCast(me, SPELL_EMINENCE, true);
                events.RescheduleEvent(EVENT_GUSTING_CRANE_KICK, 10000);
                break;
            default:
                break;
            }
        }

        void JustSummoned(Creature* summon)
        {
            switch (summon->GetEntry())
            {
            case NPC_ZARTHIK_SWARMER:
                if (me->ToTempSummon())
                    if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                        summoner->ToCreature()->AI()->JustSummoned(summon);
                summon->setFaction(16);
                summon->SetReactState(REACT_AGGRESSIVE);
                summon->AI()->DoZoneInCombat(summon, 50.0f);
                break;
            case NPC_STONE_STATUE:
                if (me->ToTempSummon())
                    if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                        summoner->ToCreature()->AI()->JustSummoned(summon);
                break;
            }
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_RE_ATTACK)
                me->SetReactState(REACT_AGGRESSIVE);
        }

        void DoRoomInCombat()
        {
            if (!me->ToTempSummon())
                return;

            if (Unit* spoil = me->ToTempSummon()->GetSummoner())
            {
                std::list<Player*> pllist;
                pllist.clear();
                GetPlayerListInGrid(pllist, spoil, 55.0f);
                if (!pllist.empty())
                {
                    for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                    {
                        (*itr)->SetInCombatWith(me);
                        me->SetInCombatWith(*itr);
                        me->AddThreat(*itr, 0.0f);
                    }
                }
            }
            else
                if (me->GetEntry() == NPC_QUILEN_GUARDIANS)
                    DoZoneInCombat(me, 10.0f);
        }

        void UpdateAI(uint32 diff)
        {
            if (spawn)
            {
                if (spawn <= diff)
                {
                    spawn = 0;
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    if (me->GetEntry() != NPC_AMBER_ENCASED_KUNCHONG && me->GetEntry() != NPC_BURIAL_URN)
                        me->SetReactState(REACT_AGGRESSIVE);
                    DoRoomInCombat();
                    if (me->GetEntry() == NPC_BURIAL_URN)
                        DoCast(me, SPELL_SPARK_OF_LIFE_CREATER, true);
                }
                else 
                    spawn -= diff;
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                //Big  
                case EVENT_SUMMON_STONE_STATUE:
                    DoCast(me, SPELL_STONE_STATUE_SUMMON);
                    events.RescheduleEvent(EVENT_SUMMON_STONE_STATUE, 12000);
                    break;
                case EVENT_MOLTEN_FIST:
                    DoCast(me, SPELL_MOLTEN_FIST);
                    events.RescheduleEvent(EVENT_MOLTEN_FIST, 15000);
                    break;
                case EVENT_SHADOW_VOLLEY:
                    DoCast(me, SPELL_SHADOW_VOLLEY);
                    events.RescheduleEvent(EVENT_SHADOW_VOLLEY, 15000);
                    break;
                case EVENT_JADE_TEMPEST:
                    DoCast(me, SPELL_JADE_TEMPEST);
                    events.RescheduleEvent(EVENT_JADE_TEMPEST, 15000);
                    break;
                case EVENT_FRACTURE:
                    DoCast(me, SPELL_FRACTURE);
                    events.RescheduleEvent(EVENT_FRACTURE, 15000);
                    break;
                case EVENT_SET_TO_BLOW:
                    if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 1, 50.0f, true))
                        me->CastSpell(target, SPELL_SET_TO_BLOW_AURA, true, 0, 0, target->GetGUID());
                    events.RescheduleEvent(EVENT_SET_TO_BLOW, 12000);
                    break;
                //Medium
                case EVENT_CRIMSON_RECOSTITUTION:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        DoCast(target, SPELL_CRIMSON_RECOSTITUTION_AT);
                    events.RescheduleEvent(EVENT_CRIMSON_RECOSTITUTION, 15000);
                    break;
                case EVENT_MATTER_SCRAMBLE:
                    DoCast(me, SPELL_MATTER_SCRAMBLE);
                    events.RescheduleEvent(EVENT_MATTER_SCRAMBLE, 18000);
                    break;
                case EVENT_TORMENT:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 50.0f, true))
                        me->CastSpell(target, SPELL_TORMENT_DUMMY, true, 0, 0, me->GetGUID());
                    events.RescheduleEvent(EVENT_TORMENT, 18000);
                    break;
                case EVENT_MOGU_RUNE_OF_POWER:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        DoCast(target, SPELL_MOGU_RUNE_OF_POWER_AT);
                    events.RescheduleEvent(EVENT_MOGU_RUNE_OF_POWER, 15000);
                    break;
                case EVENT_FORBIDDEN_MAGIC:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 10.0f, true))
                    {
                        me->StopAttack();
                        me->SetFacingToObject(target);
                        DoCast(target, SPELL_FORBIDDEN_MAGIC);
                    }
                    events.RescheduleEvent(EVENT_FORBIDDEN_MAGIC, 15000);
                    break;
                case EVENT_RESIDUE:
                    DoCast(me, SPELL_RESIDUE, false);
                    events.RescheduleEvent(EVENT_RESIDUE, 18000);
                    break;
                case EVENT_RAGE_OF_THE_EMPRESS:
                    DoCast(me, SPELL_RAGE_OF_THE_EMPRESS, false);
                    events.RescheduleEvent(EVENT_RAGE_OF_THE_EMPRESS, 18000);
                    break;
                case EVENT_WINDSTORM:
                    for (uint8 n = 0; n < 3; n++)
                        DoCast(me, SPELL_WINDSTORM_AT);
                    events.RescheduleEvent(EVENT_WINDSTORM, urand(25000, 34000));
                    break;
                //Small
                case EVENT_KW_ENRAGE:
                    DoCast(me, SPELL_KW_ENRAGE, true);
                    events.RescheduleEvent(EVENT_KW_ENRAGE, 20000);
                    break;
                case EVENT_EARTHEN_SHARD:
                    DoCastVictim(SPELL_EARTHEN_SHARD);
                    events.RescheduleEvent(EVENT_EARTHEN_SHARD, 10000);
                    break;
                case EVENT_HARDEN_FLESH:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        DoCast(target, SPELL_HARDEN_FLESH_DMG);
                    events.RescheduleEvent(EVENT_HARDEN_FLESH, 8000);
                    break;
                case EVENT_RUSH:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 60, true))
                        DoCast(target, SPELL_RUSH, true);
                    events.RescheduleEvent(EVENT_RUSH, 10000);
                    break;
                case EVENT_MANTID_SWARM:
                    for (uint8 n = 0; n < 3; n++)
                        DoCast(me, mantidswarm[n]);
                    events.RescheduleEvent(EVENT_MANTID_SWARM, 35000);
                    break;
                case EVENT_THROW_EXPLOSIVES:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        DoCast(target, SPELL_THROW_EXPLOSIVES);
                    events.RescheduleEvent(EVENT_THROW_EXPLOSIVES, 15000);
                    break;
                case EVENT_GUSTING_BOMB:
                    DoCast(me, SPELL_GUSTING_BOMB);
                    events.RescheduleEvent(EVENT_GUSTING_BOMB, 10000);
                    break;
                //Pandaren Relic box 
                case EVENT_GUSTING_CRANE_KICK:
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                    DoCast(me, SPELL_GUSTING_CRANE_KICK);
                    events.RescheduleEvent(EVENT_GUSTING_CRANE_KICK, 18000); 
                    break;
                case EVENT_KEG_TOSS:
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        DoCast(target, SPELL_KEG_TOSS);
                    events.RescheduleEvent(EVENT_KEG_TOSS, 10000);
                    break;
                case EVENT_BREATH_OF_FIRE:
                    if (me->getVictim())
                        DoCastVictim(SPELL_BREATH_OF_FIRE);
                    events.RescheduleEvent(EVENT_BREATH_OF_FIRE, 15000);
                    break;
                case EVENT_PATH_OF_BLOSSOMS:
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        DoCast(target, SPELL_PATH_OF_BLOSSOMS_J);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* killer)
        {
            if (me->ToTempSummon())
            {
                uint8 data = 0;
                switch (me->GetEntry())
                {
                //Big box
                case NPC_JUN_WEI:
                case NPC_ZU_YIN:
                case NPC_XIANG_LIN:
                case NPC_KUN_DA:
                case NPC_COMMANDER_ZAKTAR:
                case NPC_COMMANDER_IKTAL:
                case NPC_COMMANDER_NAKAZ:
                case NPC_COMMANDER_TIK:
                    data = 14;
                    break;
                //Medium box
                case NPC_MODIFIED_ANIMA_GOLEM:
                case NPC_MOGU_SHADOW_RITUALIST:
                case NPC_ZARTHIK_AMBER_PRIEST:
                case NPC_SETTHIK_WIND_WIELDER:
                    data = 4;
                    break;
                //Small box
                case NPC_ANIMATED_STONE_MOGU:
                case NPC_BURIAL_URN:
                case NPC_QUILEN_GUARDIANS:
                case NPC_SRITHIK_BOMBARDIER:
                case NPC_AMBER_ENCASED_KUNCHONG:
                case NPC_KORTHIK_WARCALLER:
                    data = 1;
                    break;
                //Pandaren Relic box
                case NPC_NAMELESS_WINDWALKER_SPIRIT:
                {
                    if (Creature* summoner = me->ToTempSummon()->GetSummoner()->ToCreature())
                        summoner->AI()->SetData(DATA_BUFF_DD, 0);
                    break;
                }
                case NPC_WISE_MISTWEAVER_SPIRIT:
                {
                    if (Creature* summoner = me->ToTempSummon()->GetSummoner()->ToCreature())
                        summoner->ToCreature()->AI()->SetData(DATA_BUFF_HEALER, 0);
                    break;
                }
                case NPC_ANCIENT_BREWMASTER_SPIRIT:
                {
                    if (Creature* summoner = me->ToTempSummon()->GetSummoner()->ToCreature())
                        summoner->ToCreature()->AI()->SetData(DATA_BUFF_TANK, 0);
                    break;
                }
                default:
                    break;
                }

                if (data)
                {
                    if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                    {
                        summoner->ToCreature()->AI()->SetData(DATA_UPDATE_POWER, data);
                        if (me->GetMap()->IsHeroic())
                            CreateUnstableSpark(summoner->GetEntry());
                    }
                }
            }
        }

        void CreateUnstableSpark(uint32 entry)
        {
            uint32 switchentry = 0;
            switch (entry)
            {
            case NPC_MOGU_SPOILS:
            case NPC_MOGU_SPOILS2:
                switchentry = GO_BIG_MANTIS_BOX;
                break;
            case NPC_MANTIS_SPOILS:
            case NPC_MANTIS_SPOILS2:
                switchentry = GO_BIG_MOGU_BOX;
                break;
            default:
                break;
            }
            if (Creature* otherspoil = me->GetCreature(*me, instance->GetGuidData(switchentry)))
            {
                float x, y;
                GetPosInRadiusWithRandomOrientation(otherspoil, float(urand(5, 20)), x, y);
                if (Creature* us = otherspoil->SummonCreature(NPC_UNSTABLE_SPARK, x, y, otherspoil->GetPositionZ()))
                    me->CastSpell(us, SPELL_UNSTABLE_SPARK_DUMMY_SPAWN);
            }
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (type == EFFECT_MOTION_TYPE)
            {
                if (pointId == SPELL_PATH_OF_BLOSSOMS_J)
                {
                    DoCast(me, SPELL_PATH_OF_BLOSSOMS_DMG, true);
                    DoCast(me, SPELL_MASS_PARALYSIS, true);
                    me->SetReactState(REACT_AGGRESSIVE);
                    DoZoneInCombat(me, 50.0f);
                    events.RescheduleEvent(EVENT_PATH_OF_BLOSSOMS, 15000);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_generic_sop_unitsAI(creature);
    }
};

//72535
class npc_stone_statue : public CreatureScript
{
public:
    npc_stone_statue() : CreatureScript("npc_stone_statue") {}

    struct npc_stone_statueAI : public ScriptedAI
    {
        npc_stone_statueAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            events.RescheduleEvent(EVENT_MOVE_STATUE, 1000);
        }

        void JustDied(Unit* killer)
        {
            me->DespawnOrUnsummon();
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (pointId == 5)
                events.RescheduleEvent(EVENT_ANIMATED_STRIKE_START, 1000);
        }

        float GetRandomAngle()
        {
            float pos = urand(0, 6);
            float ang = pos <= 5 ? pos + float(urand(1, 9)) / 10 : pos;
            return ang;
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
                case EVENT_MOVE_STATUE:
                    if (me->ToTempSummon())
                    {
                        if (Unit* spoil = me->ToTempSummon()->GetSummoner())
                        {
                            float x, y;
                            GetPosInRadiusWithRandomOrientation(spoil, urand(3, 15), x, y);
                            me->GetMotionMaster()->MoveCharge(x, y, spoil->GetPositionZ(), 7.0f, 5);
                        }
                    }
                    break;
                //Strike Mechanic
                case EVENT_ANIMATED_STRIKE_START:
                    me->SetFacingTo(GetRandomAngle());
                    DoCast(me, SPELL_ANIMATED_STRIKE_LAUNCH_V);
                    events.RescheduleEvent(EVENT_ANIMATED_STRIKE_LAUNCH, 1800);
                    break;
                case EVENT_ANIMATED_STRIKE_LAUNCH:
                    DoCast(me, SPELL_ANIMATED_STRIKE_DMG);
                    events.RescheduleEvent(EVENT_ANIMATED_STRIKE_FINISH, 4500);
                    break;
                case EVENT_ANIMATED_STRIKE_FINISH:
                    DoCast(me, SPELL_ANIMATED_STRIKE_SIT);
                    events.RescheduleEvent(EVENT_ANIMATED_STRIKE_RESET, 2000);
                    break;
                case EVENT_ANIMATED_STRIKE_RESET:
                    me->RemoveAurasDueToSpell(SPELL_ANIMATED_STRIKE_SIT);
                    events.RescheduleEvent(EVENT_MOVE_STATUE, 1000);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_stone_statueAI(creature);
    }
};

//71433
class npc_spark_of_life : public CreatureScript
{
public:
    npc_spark_of_life() : CreatureScript("npc_spark_of_life") {}

    struct npc_spark_of_lifeAI : public ScriptedAI
    {
        npc_spark_of_lifeAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }
        InstanceScript* instance;
        EventMap events;

        void Reset()
        {
            DoCast(me, SPELL_SPARK_OF_LIFE_VISUAL, true);
        }

        void JustDied(Unit* killer)
        {
            DoCast(me, SPELL_NOVA, true); //SPELL_ATTR0_CASTABLE_WHILE_DEAD
        }

        void MovementInform(uint32 type, uint32 pointId)
        {
            if (pointId == 4)
                events.RescheduleEvent(EVENT_ACTIVE, 3000);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_ACTIVE)
                {
                    if (me->ToTempSummon())
                    {
                        if (Unit* spoil = me->ToTempSummon()->GetSummoner())
                        {
                            float x, y;
                            GetPosInRadiusWithRandomOrientation(spoil, urand(3, 30), x, y);
                            me->GetMotionMaster()->MoveCharge(x, y, spoil->GetPositionZ(), 5.0f, 4);
                        }
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_spark_of_lifeAI(creature);
    }
};

//73104
class npc_unstable_spark : public CreatureScript
{
public:
    npc_unstable_spark() : CreatureScript("npc_unstable_spark") {}

    struct npc_unstable_sparkAI : public ScriptedAI
    {
        npc_unstable_sparkAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
        }
        EventMap events;

        InstanceScript* instance;

        void Reset()
        {
            events.RescheduleEvent(EVENT_ACTIVE, 500);
        }

        void JustDied(Unit* killer)
        {
            me->DespawnOrUnsummon();
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_ACTIVE)
                {
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    DoCast(me, SPELL_UNSTABLE_SPARK_SPAWN_VISUAL, true);
                    DoCast(me, SPELL_SUPERNOVA);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_unstable_sparkAI(creature);
    }
};

//220823
class go_ssop_spoils : public GameObjectScript
{
public:
    go_ssop_spoils() : GameObjectScript("go_ssop_spoils") { }

    bool OnGossipSelect(Player* pl, GameObject* go, uint32 /*uiSender*/, uint32 uiAction)
    {
        if (InstanceScript* instance = go->GetInstanceScript())
        {
            if (instance->GetBossState(DATA_SPOILS_OF_PANDARIA) == NOT_STARTED)
            {
                if (uiAction == GOSSIP_ACTION_INFO_DEF)
                {
                    pl->PlayerTalkClass->ClearMenus();
                    pl->CLOSE_GOSSIP_MENU();
                    instance->SetBossState(DATA_SPOILS_OF_PANDARIA, IN_PROGRESS);
                }
            }
        }
        return true;
    }

    bool OnGossipHello(Player* pl, GameObject* go)
    {
        if (InstanceScript* instance = go->GetInstanceScript())
        {
            if (instance->GetBossState(DATA_MALKOROK) == DONE && instance->GetBossState(DATA_SPOILS_OF_PANDARIA) == NOT_STARTED)
            {
                pl->PrepareGossipMenu(go);
                pl->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
                pl->SEND_GOSSIP_MENU(pl->GetGossipTextId(go), go->GetGUID());
            }
        }
        return true;
    }
};

//221771, 221773
class go_generic_lever : public GameObjectScript
{
public:
    go_generic_lever() : GameObjectScript("go_generic_lever") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (InstanceScript* instance = go->GetInstanceScript())
        {
            instance->HandleGameObject(instance->GetGuidData(go->GetEntry() == GO_LEVER_R ? GO_IRON_DOOR_R : GO_IRON_DOOR_L), true);
            if (instance->GetBossState(DATA_SPOILS_OF_PANDARIA) == IN_PROGRESS)
                instance->SetBossState(DATA_SPOILS_OF_PANDARIA, SPECIAL);
        }
        return false;
    }
};

//221906, 221893, 221885, 221816, 221820, 221804, 221878
class go_generic_sop_box : public GameObjectScript
{
public:
    go_generic_sop_box() : GameObjectScript("go_generic_sop_box"){}

    bool OnGossipHello(Player* player, GameObject* go)
    {
        go->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
        Position pos;
        go->GetPosition(&pos);
        if (InstanceScript* instance = go->GetInstanceScript())
        {
            if (Creature* summoner = instance->instance->GetCreature(instance->GetGuidData(go->GetEntry())))
            {
                switch (go->GetEntry())
                {
                case GO_BIG_MOGU_BOX:
                    summoner->SummonCreature(bigmoguentry[urand(0, 3)], pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                    break;
                case GO_BIG_MANTIS_BOX:
                    summoner->SummonCreature(bigmantisentry[urand(0, 3)], pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                    break;
                case GO_MEDIUM_MOGU_BOX:
                    summoner->SummonCreature(mediummoguentry[urand(0, 1)], pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                    break;
                case GO_MEDIUM_MANTIS_BOX:
                    summoner->SummonCreature(mediummantisentry[urand(0, 1)], pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                    break;
                case GO_SMALL_MOGU_BOX:
                {
                    uint8 entry = urand(0, 2);
                    switch (entry)
                    {
                    case 0:
                        for (uint8 n = 0; n < 3; n++)
                            go->SummonCreature(smallmoguentry[entry], pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                        summoner->SummonCreature(smallmoguentry[entry], pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                        break;
                    case 1:
                        summoner->SummonCreature(smallmoguentry[entry], pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                        break;
                    case 2:
                        summoner->SummonCreature(smallmoguentry[entry], pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
                        break;
                    }
                    break;
                }
                case GO_SMALL_MANTIS_BOX:
                    summoner->SummonCreature(smallmantisentry[urand(0, 2)], pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                    break;
                case GO_PANDAREN_RELIC_BOX:
                {
                    std::vector<ObjectGuid> activespoil;
                    activespoil.clear();
                    activespoil.push_back(instance->GetGuidData(GO_SMALL_MANTIS_BOX));
                    activespoil.push_back(instance->GetGuidData(GO_SMALL_MOGU_BOX));
                    if (!activespoil.empty())
                    {
                        for (std::vector<ObjectGuid>::const_iterator itr = activespoil.begin(); itr != activespoil.end(); itr++)
                        {
                            if (Creature* spoil = instance->instance->GetCreature(*itr))
                            { 
                                if (spoil->GetDistance(go) <= 50.0f)
                                {
                                    uint8 val = spoil->AI()->GetData(DATA_GET_SPIRIT_SUM_COUNT);
                                    spoil->SummonCreature(pandarenrelicentry[val], pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000);
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
                default:
                    break;
                }
            }
        }
        return false;
    }
};

//148515
class spell_shadow_volley : public SpellScriptLoader
{
public:
    spell_shadow_volley() : SpellScriptLoader("spell_shadow_volley") { }

    class spell_shadow_volley_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_shadow_volley_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster() && GetCaster()->ToTempSummon())
            {
                if (Unit* spoil = GetCaster()->ToTempSummon()->GetSummoner())
                {
                    float dmg = GetCaster()->GetMap()->IsHeroic() ? GetSpellInfo()->Effects[EFFECT_0]->BasePoints * 2 : GetSpellInfo()->Effects[EFFECT_0]->BasePoints;
                    std::list<Player*> pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, spoil, 55.0f);
                    if (!pllist.empty())
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                            GetCaster()->CastCustomSpell(SPELL_SHADOW_VOLLEY_D, SPELLVALUE_BASE_POINT0, dmg, *itr, true);
                }
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_shadow_volley_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_shadow_volley_SpellScript();
    }
};

//148518
class spell_molten_fist : public SpellScriptLoader
{
public:
    spell_molten_fist() : SpellScriptLoader("spell_molten_fist") { }

    class spell_molten_fist_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_molten_fist_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster() && GetCaster()->ToTempSummon())
            {
                if (Unit* spoil = GetCaster()->ToTempSummon()->GetSummoner())
                {
                    float dmg = GetCaster()->GetMap()->IsHeroic() ? GetSpellInfo()->Effects[EFFECT_0]->BasePoints * 2 : GetSpellInfo()->Effects[EFFECT_0]->BasePoints;
                    std::list<Player*> pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, spoil, 55.0f);
                    if (!pllist.empty())
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                            GetCaster()->CastCustomSpell(SPELL_MOLTEN_FIST_D, SPELLVALUE_BASE_POINT0, dmg, *itr, true);
                }
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_molten_fist_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_molten_fist_SpellScript();
    }
};

//148513
class spell_fracture : public SpellScriptLoader
{
public:
    spell_fracture() : SpellScriptLoader("spell_fracture") { }

    class spell_fracture_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_fracture_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster())
            {
                float dmg = GetCaster()->GetMap()->IsHeroic() ? GetSpellInfo()->Effects[EFFECT_0]->BasePoints * 2 : GetSpellInfo()->Effects[EFFECT_0]->BasePoints;
                std::list<Player*> pllist;
                pllist.clear();
                GetPlayerListInGrid(pllist, GetCaster(), 30.0f);
                if (!pllist.empty())
                    for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                        GetCaster()->CastCustomSpell(SPELL_FRACTURE_D, SPELLVALUE_BASE_POINT0, dmg, *itr, true);
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_fracture_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_fracture_SpellScript();
    }
};

//148582
class spell_jade_tempest : public SpellScriptLoader
{
public:
    spell_jade_tempest() : SpellScriptLoader("spell_jade_tempest") { }

    class spell_jade_tempest_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_jade_tempest_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster())
            {
                float dmg = GetCaster()->GetMap()->IsHeroic() ? GetSpellInfo()->Effects[EFFECT_0]->BasePoints * 2 : GetSpellInfo()->Effects[EFFECT_0]->BasePoints;
                std::list<Player*> pllist;
                pllist.clear();
                GetPlayerListInGrid(pllist, GetCaster(), 50.0f);
                if (!pllist.empty())
                    for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                        GetCaster()->CastCustomSpell(SPELL_JADE_TEMPEST_D, SPELLVALUE_BASE_POINT0, dmg, *itr, true);
            }

        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_jade_tempest_SpellScript::HandleAfterCast);
        };
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_jade_tempest_SpellScript();
    }
};

//146222
class spell_breath_of_fire : public SpellScriptLoader
{
public:
    spell_breath_of_fire() : SpellScriptLoader("spell_breath_of_fire") { }

    class spell_breath_of_fire_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_breath_of_fire_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster() && GetTarget())
            {
                GetCaster()->CastSpell(GetTarget(), SPELL_BREATH_OF_FIRE_DMG, true);
                if (GetTarget()->HasAura(SPELL_KEG_TOSS_DMG))
                    GetCaster()->CastSpell(GetTarget(), SPELL_BREATH_OF_FIRE_TR, true);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_breath_of_fire_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_breath_of_fire_AuraScript();
    }
};

//146180
class spell_gusting_crane_kick : public SpellScriptLoader
{
public:
    spell_gusting_crane_kick() : SpellScriptLoader("spell_gusting_crane_kick") { }

    class spell_gusting_crane_kick_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_gusting_crane_kick_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster())
                GetCaster()->CastSpell(GetCaster(), SPELL_GUSTING_CRANE_KICK_DMG, true);
        }

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                {
                    GetCaster()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                    GetCaster()->ToCreature()->SetReactState(REACT_AGGRESSIVE);
                    GetCaster()->ToCreature()->AI()->DoZoneInCombat(GetCaster()->ToCreature(), 50.0f);
                }
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_gusting_crane_kick_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_gusting_crane_kick_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_gusting_crane_kick_AuraScript();
    }
};

//145987
class spell_set_to_blow : public SpellScriptLoader
{
public:
    spell_set_to_blow() : SpellScriptLoader("spell_set_to_blow") { }

    class spell_set_to_blow_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_set_to_blow_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            Unit* target = GetTarget();
            if (!target)
                return;

            if (Aura* aura = aurEff->GetBase())
            {
                uint8 stack = aura->GetStackAmount();
                if (!stack)
                {
                    target->RemoveAura(SPELL_SET_TO_BLOW_VISUAL_1);
                    return;
                }

                switch (stack)
                {
                    case 1:
                        target->RemoveAura(SPELL_SET_TO_BLOW_VISUAL_2);
                        target->CastSpell(target, SPELL_SET_TO_BLOW_VISUAL_1, true);
                        break;
                    case 2:
                        target->RemoveAura(SPELL_SET_TO_BLOW_VISUAL_3);
                        target->CastSpell(target, SPELL_SET_TO_BLOW_VISUAL_2, true);
                        break;
                    case 3:
                        target->RemoveAura(SPELL_SET_TO_BLOW_VISUAL_4);
                        target->CastSpell(target, SPELL_SET_TO_BLOW_VISUAL_3, true);
                        break;
                    case 4:
                        target->CastSpell(target, SPELL_SET_TO_BLOW_VISUAL_4, true);
                        break;
                }
            }
        }

        void OnApply(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (Aura* aura = aurEff->GetBase())
                aura->SetStackAmount(4);
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {
            Unit* target = GetTarget();
            if (!target)
                return;

            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
            {
                if (Aura* aura = aurEff->GetBase())
                {
                    uint8 stack = aura->GetStackAmount();
                    if (!stack)
                        return;

                    for (uint8 i = 0; i < stack; i++)
                        target->CastSpell(target, SPELL_SET_TO_BLOW_DMG, true);
                }
            }
            target->RemoveAura(SPELL_SET_TO_BLOW_VISUAL_1);
            target->RemoveAura(SPELL_SET_TO_BLOW_VISUAL_2);
            target->RemoveAura(SPELL_SET_TO_BLOW_VISUAL_3);
            target->RemoveAura(SPELL_SET_TO_BLOW_VISUAL_4);
            target->RemoveAura(146364);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_set_to_blow_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            OnEffectApply += AuraEffectApplyFn(spell_set_to_blow_AuraScript::OnApply, EFFECT_0, SPELL_AURA_OVERRIDE_SPELLS, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_set_to_blow_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_OVERRIDE_SPELLS, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_set_to_blow_AuraScript();
    }
};

//148762
class spell_pheromone_cloud : public SpellScriptLoader
{
    public:
        spell_pheromone_cloud() : SpellScriptLoader("spell_pheromone_cloud") { }

        class spell_pheromone_cloud_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_pheromone_cloud_AuraScript);

            void HandlePeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (Creature* caster = GetCaster()->ToCreature())
                    if (Unit* pTarget = caster->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        caster->CastSpell(pTarget, SPELL_PHEROMONE_CLOUD_DMG, true);
            }

            void Register()
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_pheromone_cloud_AuraScript::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_pheromone_cloud_AuraScript();
        }
};

//145812
class spell_rage_of_the_empress : public SpellScriptLoader
{
    public:
        spell_rage_of_the_empress() : SpellScriptLoader("spell_rage_of_the_empress") { }

        class spell_rage_of_the_empress_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_rage_of_the_empress_SpellScript);

            float bp0;
            bool Load()
            {
                bp0 = 0;
                return true;
            }

            void FilterTargetsDamage(std::list<WorldObject*>& targets)
            {
                Unit* caster = GetCaster();
                targets.remove(caster);

                if (targets.empty())
                    return;

                SpellInfo const* Spell = sSpellMgr->GetSpellInfo(SPELL_RAGE_OF_THE_EMPRESS_AURA);
                bp0 = Spell->GetEffect(EFFECT_0, caster->GetSpawnMode())->BasePoints / targets.size();
            }

            void HandleOnHit()
            {
                if (!GetCaster())
                    return;

                if (!GetHitUnit())
                    return;

                GetCaster()->CastCustomSpell(GetHitUnit(), SPELL_RAGE_OF_THE_EMPRESS_AURA, &bp0, NULL, NULL, false);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_rage_of_the_empress_SpellScript::FilterTargetsDamage, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
                OnHit += SpellHitFn(spell_rage_of_the_empress_SpellScript::HandleOnHit);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_rage_of_the_empress_SpellScript();
        }
};

// Gusting Bomb - 145712
class spell_boss_gusting_bomb : public SpellScriptLoader
{
    public:
        spell_boss_gusting_bomb() : SpellScriptLoader("spell_boss_gusting_bomb") { }

        class spell_spell_boss_gusting_bomb_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_spell_boss_gusting_bomb_SpellScript);

            void HandleOnCast()
            {
                if(Unit* caster = GetCaster())
                {
                    if(!caster->ToCreature() || !caster->ToCreature()->AI())
                        return;

                    Unit* target = caster->ToCreature()->AI()->SelectTarget(SELECT_TARGET_FARTHEST, 0, 50.0f, true);
                    if(!target)
                        return;

                    uint32 count = uint32(caster->GetDistance(target) / 2);
                    float angle = caster->GetAngle(target);
                    target->GetPosition(&GetSpell()->visualPos);

                    caster->CastSpell(target, SPELL_GUSTING_BOMB_AOE_DMG, true);
                    if(count > 0)
                    {
                        for(uint32 j = 1; j < count + 1; ++j)
                        {
                            uint32 distanceNext = j * 2;
                            float destx = caster->GetPositionX() + distanceNext * std::cos(angle);
                            float desty = caster->GetPositionY() + distanceNext * std::sin(angle);
                            caster->CastSpell(destx, desty, caster->GetPositionZ(), SPELL_GUSTING_BOMB_AT, true);
                        }
                    }
                }
            }

            void Register()
            {
                OnCast += SpellCastFn(spell_spell_boss_gusting_bomb_SpellScript::HandleOnCast);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_spell_boss_gusting_bomb_SpellScript();
        }
};

//145288 - Matter Scramble
class spell_boss_matter_scramble : public SpellScriptLoader
{
public:
    spell_boss_matter_scramble() : SpellScriptLoader("spell_boss_matter_scramble") { }

    class spell_boss_matter_scramble_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_boss_matter_scramble_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if(Unit* caster = GetCaster())
            {
                std::vector<Position>* positions = GetAura()->GetDstVector();
                for (std::vector<Position>::const_iterator itr = positions->begin(); itr != positions->end(); ++itr)
                    caster->SendSpellCreateVisual(GetSpellInfo(), &(*itr), NULL, 1, 32969);
            }
        }

        void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
        {
            if(Unit* caster = GetCaster())
            {
                std::vector<Position>* positions = GetAura()->GetDstVector();
                for (std::vector<Position>::const_iterator itr = positions->begin(); itr != positions->end(); ++itr)
                    caster->CastSpell((*itr).GetPositionX(), (*itr).GetPositionY(), (*itr).GetPositionZ(), 145369, true, NULL, aurEff);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_boss_matter_scramble_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_boss_matter_scramble_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    class spell_boss_matter_scramble_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_boss_matter_scramble_SpellScript);

        void HandleOnCast()
        {
            if(Unit* caster = GetCaster())
            {
                uint32 count = caster->GetMap()->Is25ManRaid() ? 3 : 2;
                Position firstCenter, secondCenter, work;
                firstCenter.Relocate(1691.27f, -5137.44f, -289.92f);
                secondCenter.Relocate(1612.71f, -5181.84f, -289.92f);
                if(caster->GetDistance(firstCenter) > caster->GetDistance(secondCenter))
                    work.Relocate(secondCenter);
                else
                    work.Relocate(firstCenter);

                float angle = float(rand_norm())*static_cast<float>(2*M_PI);
                float angleNext = (2*M_PI) / count;
                for (uint32 i = 0; i < count; ++i)
                {
                    Position second;
                    float distance = frand(5.0f,12.0f);

                    float x = work.GetPositionX() + distance * std::cos(angle);
                    float y = work.GetPositionY() + distance * std::sin(angle);
                    float z = work.GetPositionZ();

                    Trinity::NormalizeMapCoord(x);
                    Trinity::NormalizeMapCoord(y);
                    second.Relocate(x, y, z);
                    GetSpell()->AddDst(&second);

                    caster->SendSpellCreateVisual(GetSpellInfo(), &second, NULL, 1, 33807);
                    angle += angleNext;
                }
            }
        }

        void Register()
        {
            OnCast += SpellCastFn(spell_boss_matter_scramble_SpellScript::HandleOnCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_boss_matter_scramble_SpellScript();
    }

    AuraScript* GetAuraScript() const
    {
        return new spell_boss_matter_scramble_AuraScript();
    }
};

//145369 - Matter Scramble
class spell_boss_matter_scramble_filter : public SpellScriptLoader
{
    public:
        spell_boss_matter_scramble_filter() : SpellScriptLoader("spell_boss_matter_scramble_filter") { }

    class spell_boss_matter_scramble_filter_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_boss_matter_scramble_filter_SpellScript);

        void FilterTargets(std::list<WorldObject*>& unitList)
        {
            if(unitList.empty())
            {
                Position const* pos = GetExplTargetDest();
                if(Unit* caster = GetCaster())
                    caster->CastSpell(pos->GetPositionX(), pos->GetPositionY(), pos->GetPositionZ(), 145393, true);
            }
        }

        void HandleOnHit()
        {
            if(Unit* target = GetHitUnit())
            {
                Position firstCenter, secondCenter, work, second;
                Position const* first = GetExplTargetDest();
                uint32 count = target->GetMap()->Is25ManRaid() ? 3 : 2;

                firstCenter.Relocate(1691.27f, -5137.44f, -289.92f);
                secondCenter.Relocate(1612.71f, -5181.84f, -289.92f);
                if(target->GetDistance(firstCenter) > target->GetDistance(secondCenter))
                    work.Relocate(secondCenter);
                else
                    work.Relocate(firstCenter);

                float angle = work.GetAngle(first) + (2*M_PI) / count;
                float distance = work.GetExactDist(first);

                float x = work.GetPositionX() + distance * std::cos(angle);
                float y = work.GetPositionY() + distance * std::sin(angle);
                float z = work.GetPositionZ();

                Trinity::NormalizeMapCoord(x);
                Trinity::NormalizeMapCoord(y);
                second.Relocate(x, y, z);

                target->SendPlaySpellVisualKit(35643, 0);
                target->NearTeleportTo(second.GetPositionX(), second.GetPositionY(), second.GetPositionZ(), target->GetOrientation(), true);
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_boss_matter_scramble_filter_SpellScript::HandleOnHit);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_boss_matter_scramble_filter_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        };
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_boss_matter_scramble_filter_SpellScript();
    }
};

// Staff of Resonating Water - 146098
class spell_spoils_staff_of_resonating_water : public SpellScriptLoader
{
public:
    spell_spoils_staff_of_resonating_water() : SpellScriptLoader("spell_spoils_staff_of_resonating_water") { }

    class spell_spoils_staff_of_resonating_water_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_spoils_staff_of_resonating_water_SpellScript);

        void HandleBeforeCast()
        {
            if(Unit* caster = GetOriginalCaster())
            {
                float x, y;
                WorldLocation loc;
                caster->GetNearPoint2D(x, y, 40.0f, caster->GetOrientation());
                loc.Relocate(x, y, caster->GetPositionZ());
                GetSpell()->destAtTarget = loc;
            }
        }

        void Register()
        {
            BeforeCast += SpellCastFn(spell_spoils_staff_of_resonating_water_SpellScript::HandleBeforeCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_spoils_staff_of_resonating_water_SpellScript();
    }
};

//145685
class spell_unstable_defense_system_dummy : public SpellScriptLoader
{
public:
    spell_unstable_defense_system_dummy() : SpellScriptLoader("spell_unstable_defense_system_dummy") { }

    class spell_unstable_defense_system_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_unstable_defense_system_dummy_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetTarget())
            {
                SpellInfo const* spell = sSpellMgr->GetSpellInfo(SPELL_UNSTABLE_DEFENSE_SYSTEMS_D);
                int32 amount = spell->GetEffect(0)->BasePoints + int32(spell->GetEffect(0)->BasePoints * 0.4f * aurEff->GetTickNumber());
                GetTarget()->CastCustomSpell(SPELL_UNSTABLE_DEFENSE_SYSTEMS_D, SPELLVALUE_BASE_POINT0, amount, GetTarget(), true);
                if (!GetTarget()->GetMap()->IsDungeon())
                    GetTarget()->RemoveAurasDueToSpell(SPELL_UNSTABLE_DEFENSE_SYSTEMS);
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_unstable_defense_system_dummy_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_unstable_defense_system_dummy_AuraScript();
    }
};

//142524
class spell_spoils_encapsulated_pheromones : public SpellScriptLoader
{
public:
    spell_spoils_encapsulated_pheromones() : SpellScriptLoader("spell_spoils_encapsulated_pheromones") { }
    
    class spell_spoils_encapsulated_pheromones_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_spoils_encapsulated_pheromones_AuraScript);
        
        void OnTick(AuraEffect const* aurEff)
        {
            if (Unit* caster = GetCaster())
            {
                if (Unit* target = caster->ToCreature()->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 60.0f, true))
                {
                    Position savePos;
                    uint32 count = uint32(caster->GetDistance(target) / 0.05f);
                    float angle = caster->GetAngle(target);

                    if (count > 0)
                    {
                        for (uint32 j = 1; j < count + 1; ++j)
                        {
                            uint32 distanceNext = j * 0.05f;
                            float destx = caster->GetPositionX() + distanceNext * std::cos(angle);
                            float desty = caster->GetPositionY() + distanceNext * std::sin(angle);
                            savePos.Relocate(destx, desty, caster->GetPositionZ());
                            caster->SendSpellCreateVisual(GetSpellInfo(), &savePos, NULL, 1, 34287);
                        }
                    }
                    caster->CastSpell(target, SPELL_ENCAPSULATED_PHEROMONES_AT, true);
                }
            }
        }
        
        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_spoils_encapsulated_pheromones_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };
    
    AuraScript* GetAuraScript() const
    {
        return new spell_spoils_encapsulated_pheromones_AuraScript();
    }
};

//142942
class spell_torment : public SpellScriptLoader
{
public:
    spell_torment() : SpellScriptLoader("spell_torment") { }

    class spell_torment_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_torment_SpellScript);

        void HandleOnHit()
        {
            if (GetHitUnit())
                if (Unit* ocaster = GetOriginalCaster())
                    GetHitUnit()->CastSpell(GetHitUnit(), SPELL_TORMENT_MAIN, true, 0, 0, ocaster->GetGUID());
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_torment_SpellScript::HandleOnHit);
        };
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_torment_SpellScript();
    }
};

//142983
class spell_torment_periodic : public SpellScriptLoader
{
public:
    spell_torment_periodic() : SpellScriptLoader("spell_torment_periodic") { }

    class spell_torment_periodic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_torment_periodic_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetTarget() && GetCaster())
            {
                float amount = aurEff->GetAmount() + (aurEff->GetAmount() * 0.1f * aurEff->GetTickNumber());
                GetCaster()->CastCustomSpell(GetTarget(), SPELL_TORMENT_DMG, &amount, 0, 0, true, 0, aurEff);
            }
        }

        void HandleEffectRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_ENEMY_SPELL)
            {
                if (GetCaster() && GetTarget())
                {
                    if (GetCaster()->isAlive())
                    {
                        float dist = 5.0f;
                        std::list<Player*>pllist;
                        for (uint8 n = 0; n < 10; n++)
                        {
                            pllist.clear();
                            GetPlayerListInGrid(pllist, GetTarget(), dist);
                            if (!pllist.empty())
                            {
                                for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                                {
                                    if (GetTarget()->GetGUID() != (*itr)->GetGUID() && !(*itr)->HasAura(SPELL_TORMENT_MAIN))
                                    {
                                        GetTarget()->CastSpell(*itr, SPELL_TORMENT_DUMMY, true, 0, 0, GetCaster()->GetGUID());
                                        return;
                                    }
                                }
                            }
                            dist += 5.0f;
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_torment_periodic_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            OnEffectRemove += AuraEffectRemoveFn(spell_torment_periodic_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };


    AuraScript* GetAuraScript() const
    {
        return new spell_torment_periodic_AuraScript();
    }
};

//145230
class spell_forbidden_magic : public SpellScriptLoader
{
public:
    spell_forbidden_magic() : SpellScriptLoader("spell_forbidden_magic") { }

    class spell_forbidden_magic_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_forbidden_magic_AuraScript);

        void HandleEffectRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster() && GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
                if (GetCaster()->ToCreature())
                    GetCaster()->ToCreature()->AI()->DoAction(ACTION_RE_ATTACK);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_forbidden_magic_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        }
    };


    AuraScript* GetAuraScript() const
    {
        return new spell_forbidden_magic_AuraScript();
    }
};

//145489
class spell_stone_statue_summon : public SpellScriptLoader
{
public:
    spell_stone_statue_summon() : SpellScriptLoader("spell_stone_statue_summon") { }

    class spell_stone_statue_summon_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_stone_statue_summon_SpellScript);

        void HandleAfterCast()
        {
            if (GetCaster() && GetCaster()->ToCreature() && GetCaster())
            {
                if (Unit* spoil = GetCaster()->ToTempSummon()->GetSummoner())
                {
                    for (uint8 n = 0; n < 2; n++)
                    {
                        float x, y;
                        GetPosInRadiusWithRandomOrientation(spoil, float(urand(5.0f, 20.0f)), x, y);
                        GetCaster()->SummonCreature(NPC_STONE_STATUE, x, y, spoil->GetPositionZ());
                    }
                }
            }
        }

        void Register()
        {
            AfterCast += SpellCastFn(spell_stone_statue_summon_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_stone_statue_summon_SpellScript();
    }
};

//145716
class spell_gusting_bomb_dmg : public SpellScriptLoader
{
public:
    spell_gusting_bomb_dmg() : SpellScriptLoader("spell_gusting_bomb_dmg") { }

    class spell_gusting_bomb_dmg_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_gusting_bomb_dmg_SpellScript);

        void HandleOnHit()
        {
            SetHitDamage(GetHitDamage() / 10);
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_gusting_bomb_dmg_SpellScript::HandleOnHit);
        };
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_gusting_bomb_dmg_SpellScript();
    }
};

//145998
class spell_strength_of_the_stone : public SpellScriptLoader
{
public:
    spell_strength_of_the_stone() : SpellScriptLoader("spell_strength_of_the_stone") { }

    class spell_strength_of_the_stone_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_strength_of_the_stone_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetTarget() && GetTarget()->ToCreature() && GetTarget()->ToTempSummon())
            {
                if (Unit* spoil = GetTarget()->ToTempSummon()->GetSummoner())
                {
                    std::list<Creature*>sslist;
                    sslist.clear();
                    GetCreatureListWithEntryInGrid(sslist, spoil, NPC_STONE_STATUE, 55.0f);
                    uint8 size = sslist.size();
                    if (Aura* aura = GetTarget()->GetAura(SPELL_STRENGTH_OF_THE_STONE))
                        aura->SetStackAmount(size + 1);
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_strength_of_the_stone_AuraScript::OnTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_strength_of_the_stone_AuraScript();
    }
};

class SuperNovaFilter
{
public:
    SuperNovaFilter(Unit* caster) : _caster(caster){}

    bool operator()(WorldObject* target)
    {
        if (target->GetPositionZ() < -281.0f && target->GetExactDist2d(_caster) <= 55.0f)
            return false;
        return true;
    }
private:
    Unit* _caster;
};

//146815
class spell_supernova : public SpellScriptLoader
{
public:
    spell_supernova() : SpellScriptLoader("spell_supernova") { }

    class spell_supernova_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_supernova_SpellScript);

        void FilterTargets(std::list<WorldObject*>&unitList)
        {
            if (GetCaster() && GetCaster()->ToTempSummon())
                if (Unit* spoil = GetCaster()->ToTempSummon()->GetSummoner())
                    unitList.remove_if(SuperNovaFilter(spoil));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_supernova_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_supernova_SpellScript();
    }
};

//142695
class spell_spark_of_life_creater : public SpellScriptLoader
{
public:
    spell_spark_of_life_creater() : SpellScriptLoader("spell_spark_of_life_creater") { }

    class spell_spark_of_life_creater_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_spark_of_life_creater_SpellScript);

        void CheckTarget(SpellEffIndex effIndex)
        {
            if (GetCaster() && GetExplTargetDest())
            {
                float spawnx, spawny, spawnz;
                GetExplTargetDest()->GetPosition(spawnx, spawny, spawnz);
                if (GetCaster()->ToTempSummon())
                {
                    if (Unit* spoil = GetCaster()->ToTempSummon()->GetSummoner())
                    {
                        if (Creature* sparkoflife = spoil->SummonCreature(NPC_SPARK_OF_LIFE, spawnx, spawny, spawnz, 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 4000))
                        {
                            float destx, desty;
                            GetPosInRadiusWithRandomOrientation(spoil, urand(3, 30), destx, desty);
                            sparkoflife->AddAura(SPELL_PULSE, sparkoflife);
                            sparkoflife->GetMotionMaster()->MoveCharge(destx, desty, spoil->GetPositionZ(), 5.0f, 4);
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_spark_of_life_creater_SpellScript::CheckTarget, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_spark_of_life_creater_SpellScript();
    }
};


void AddSC_boss_spoils_of_pandaria()
{
    new npc_ssop_spoils();
    new npc_generic_spoil();
    new npc_lever();
    new npc_lift_hook();
    new npc_generic_sop_units();
    new npc_stone_statue();
    new npc_spark_of_life();
    new npc_unstable_spark();
    new go_ssop_spoils();
    new go_generic_lever();
    new go_generic_sop_box();
    new spell_shadow_volley();
    new spell_molten_fist();
    new spell_fracture();
    new spell_jade_tempest();
    new spell_breath_of_fire();
    new spell_gusting_crane_kick();
    new spell_set_to_blow();
    new spell_pheromone_cloud();
    new spell_rage_of_the_empress();
    new spell_boss_gusting_bomb();
    new spell_boss_matter_scramble();
    new spell_boss_matter_scramble_filter();
    new spell_spoils_staff_of_resonating_water();
    new spell_unstable_defense_system_dummy();
    new spell_spoils_encapsulated_pheromones();
    new spell_torment();
    new spell_torment_periodic();
    new spell_forbidden_magic();
    new spell_stone_statue_summon();
    new spell_gusting_bomb_dmg();
    new spell_strength_of_the_stone();
    new spell_supernova();
    new spell_spark_of_life_creater();
}
