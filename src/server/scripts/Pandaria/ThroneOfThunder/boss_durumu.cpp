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
    SPELL_GAZE                    = 134029,
    SPELL_HARD_STARE              = 133765,
    SPELL_OBLITERATE              = 137747,
    SPELL_LINGERING_GAZE          = 138467,
    SPELL_LINGERING_GAZE_TR_M     = 133792, //Create AT
    SPELL_LINGERING_GAZE_DMG      = 134040,
    SPELL_LINGERING_GAZE_TARGET   = 134626,
    SPELL_ARTERIAL_CUT            = 133768,
    SPELL_FORCE_OF_WILL_KNOCK_B   = 136413,
    SPELL_FORCE_OF_WILL_DUMMY     = 136932,
    SPELL_DURUMU_EYE_TRAIL_ORANGE = 141006,
    SPELL_DURUMU_EYE_TRAIL_RED    = 141008,
    SPELL_DURUMU_EYE_TRAIL_BLUE   = 141009,
    SPELL_DURUMU_EYE_TRAIL_YELLOW = 141010,
    SPELL_BRIGHT_LIGHT_DURUMU     = 136121,
    SPELL_LIFE_DRAIN_DUMMY        = 133795,
    SPELL_LIFE_DRAIN_WARNING      = 140866,
    SPELL_LIFE_DRAIN_STUN         = 137727,
    SPELL_LIFE_DRAIN_DMG          = 133798,
    SPELL_LIFE_DRAIN_HEAL         = 133807,

    //Red
    SPELL_INFRARED_LIGHT_P_T_AURA = 133731, //player target aura
    SPELL_INFRARED_LIGHT_C_T_AURA = 136120, //creature target aura
    SPELL_INFRARED_LIGHT_BEAM     = 134123, //channel beam
    SPELL_INFRARED_LIGHT_CONE     = 133734, //channel cone
    SPELL_INFRARED_LIGHT_CONE_DMG = 133732,
    SPELL_INFRARED_LIGHT_EXPLOSE  = 133733,
    SPELL_BURNING_EYE_FOUND       = 137002,
    SPELL_RED_FOG_INVISIBILITY    = 136116,
    SPELL_SUMMON_RED_FOG          = 136128,

    //Blue
    SPELL_BLUE_RAY_P_T_AURA       = 133675, //player target aura
    SPELL_BLUE_RAY_C_T_AURA       = 136119, //creature target aura
    SPELL_BLUE_RAY_BEAM           = 134122, //channel beam
    SPELL_BLUE_RAY_CONE           = 133672, //channel cone
    SPELL_BLUE_RAY_CONE_DMG       = 133677,
    SPELL_BLUE_RAY_EXPLOSE        = 133678,
    SPELL_COLD_EYE_FOUND          = 137054,
    SPELL_BLUE_FOG_INVISIBILITY   = 136118,
    SPELL_SUMMON_BLUE_FOG         = 136130,

    //Yellow
    SPELL_BRIGHT_LIGHT_P_T_AURA   = 133737, //player target  aura
    SPELL_BRIGHT_LIGHT_C_T_AURA   = 136121, //creature target aura
    SPELL_BRIGHT_LIGHT_BEAM       = 134124, //channel beam
    SPELL_BRIGHT_LIGHT_CONE       = 133740, //channel cone
    SPELL_BRIGHT_LIGHT_CONE_DMG   = 133738,
    SPELL_BRIGHT_LIGHT_EXPLOSE    = 133739,
    SPELL_YELLOW_FOG_INVISIBILITY = 136117,

    //Crimson Fog
    SPELL_CAUSTIC_SPIKE           = 136154, //While revealed
    SPELL_CRIMSON_BLOOM           = 136122,

    //Azure Fog
    SPELL_ICY_GRASP               = 136177,
    SPELL_ICY_GRASP_AURA          = 136179,
    SPELL_FLASH_FREEZE            = 136124, //explose them die

    //Disentegration Phase
    SPELL_MIND_DAGGERS_AURA       = 139108,
    SPELL_MIND_DAGGERS_DMG        = 139107,
    SPELL_DISINTEGRATION_LASER    = 133776,
    SPELL_DISINTEGRATION_LASER_AT = 133778,
    SPELL_DISINTEGRATION_LASER_P  = 134169, //Prepare
    SPELL_DISINTEGRATION_LASER_S  = 133775, //Summon eyebeam target
    SPELL_MAZE_STARTS_HERE        = 140911,
    SPELL_DURUMU_SPAWN            = 139089,
    SPELL_MAZE_COLLECTED          = 140911,

    //Thunder King Raid - Durumu - Whole Room Maze - Whole Slice
    SPELL_TKR_WHOLE_SLICE_X_1     = 136553,

    //Whole Room Slice
    SPELL_WHOLE_ROOM_SLICE_1      = 136232,
    SPELL_WHOLE_ROOM_SLICE_2      = 136233,
    SPELL_WHOLE_ROOM_SLICE_3      = 136234,
    SPELL_WHOLE_ROOM_SLICE_4      = 136235,
    SPELL_WHOLE_ROOM_SLICE_5      = 136236,
    SPELL_WHOLE_ROOM_SLICE_6      = 136237,
    SPELL_WHOLE_ROOM_SLICE_7      = 136238,
    SPELL_WHOLE_ROOM_SLICE_8      = 136239,
    SPELL_WHOLE_ROOM_SLICE_9      = 136240,
    SPELL_WHOLE_ROOM_SLICE_10     = 136241,
    SPELL_WHOLE_ROOM_SLICE_11     = 136242,
    SPELL_WHOLE_ROOM_SLICE_12     = 136243,
    SPELL_WHOLE_ROOM_SLICE_13     = 136244,
};

enum sEvents
{
    //Normal phase
    EVENT_HARD_STARE = 1,
    EVENT_LINGERING_GAZE_PREPARE,
    EVENT_LINGERING_GAZE,
    EVENT_FORCE_OF_WILL_PREPARE,
    EVENT_FORCE_OF_WILL,

    EVENT_PREPARE_TO_MIST,
    EVENT_CREATE_MIST,
    EVENT_ADVANCE_PHASE,
    //

    EVENT_MOVE_TO_POINT,
    EVENT_START_MOVE,
    EVENT_RESTART_MOVING,
    EVENT_COLORBLIND,
    EVENT_PREPARE_BEAM,
    EVENT_CREATE_CONE,
    EVENT_SEARCHER,
    //Fog events
    EVENT_CAUSTIC_SPIKE,
    EVENT_DISINTEGRATION_BEAM,
    EVENT_DISINTEGRATION_BEAM_P,
    EVENT_OPEN_WAY_IN_MIST,
    EVENT_CLOSE_SAFE_ZONE_IN_MIST,

    //After Disintegration phase
    EVENT_LIFE_DRAIN_PREPARE,
    EVENT_LIFE_DRAIN,
    EVENT_LIFE_DRAIN_LAUNCH,
};

enum sActions
{
    ACTION_LINGERING_GAZE = 1,
    ACTION_CREATE_CONE,
    ACTION_IN_CONE,
    ACTION_NOT_IN_CONE,
    ACTION_LAUNCH_ROTATE,
};

enum Phase
{
    PHASE_NULL,
    PHASE_NORMAL,
    PHASE_COLORBLIND,
    PHASE_DISINTEGRATION_BEAM,
    PHASE_ADVANCE,
};

uint32 colorblindeyelist[3] =
{
    NPC_RED_EYE,
    NPC_BLUE_EYE,
    NPC_YELLOW_EYE,
};

Position Durumucenterpos = { 5895.52f, 4512.58f, -6.27f };
Position Eyebeamtargetpos = { 5965.542f, 4512.609f, -2.433161f };

enum CreatureText
{
    SAY_ENTERCOMBAT          = 1, //Узрите силу Бездны!                                    35336
    SAY_KILL_PLAYER          = 2, //Наблюдайте за своей смертью.                           35345
    SAY_FORCE_OF_WILL        = 3, //Я слежу за вами…                                       35344
    SAY_COLORBLIND           = 4, //Туманы хранят много секретов, если знать, где искать…  35343
    SAY_DISINTEGRATION_START = 5, //Смотрите под ноги…                                     35342
    SAY_DIE                  = 6, //Бездна зовёт меня…                                     35338
};

Position triggerspawnpos[12] =
{
    //minimap - north -> west                                            // From center
    { 5897.926f, 4514.828f, -6.277899f, 0.000000f }, // 0                // 0
    { 5897.926f, 4514.828f, -6.277899f, 0.523598f }, // 1    maze right  // 1
    { 5897.926f, 4514.828f, -6.277899f, 1.047198f }, // 2                // 2

    { 5897.926f, 4514.828f, -6.277899f, 1.570796f }, // 3
    { 5897.926f, 4514.828f, -6.277899f, 2.094395f }, // 4
    { 5897.926f, 4514.828f, -6.277899f, 2.617994f }, // 5

    { 5897.926f, 4514.828f, -6.277899f, 3.141593f }, // 6
    { 5897.926f, 4514.828f, -6.277899f, 3.665191f }, // 7
    { 5897.926f, 4514.828f, -6.277899f, 4.188790f }, // 8

    { 5897.926f, 4514.828f, -6.277899f, 4.712389f }, // 9                // 2
    { 5897.926f, 4514.828f, -6.277899f, 5.235987f }, // 10   maze left   // 1
    { 5897.926f, 4514.828f, -6.277899f, 5.759586f }, // 11               // 0
};

uint32 wholeroomslice[11] =
{
    SPELL_WHOLE_ROOM_SLICE_3,
    SPELL_WHOLE_ROOM_SLICE_4,
    SPELL_WHOLE_ROOM_SLICE_5,
    SPELL_WHOLE_ROOM_SLICE_6,
    SPELL_WHOLE_ROOM_SLICE_7,
    SPELL_WHOLE_ROOM_SLICE_8,
    SPELL_WHOLE_ROOM_SLICE_9,
    SPELL_WHOLE_ROOM_SLICE_10,
    SPELL_WHOLE_ROOM_SLICE_11,
    SPELL_WHOLE_ROOM_SLICE_12,
    SPELL_WHOLE_ROOM_SLICE_13,
};

class _TankFilter
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

class VictimFilter
{
public:
    VictimFilter(ObjectGuid victimguid) : _victimguid(victimguid){}

    bool operator()(WorldObject* unit)
    {
        if (unit->GetGUID() == _victimguid)
            return true;
        return false;
    }
private:
    ObjectGuid _victimguid;
};

class LingeringGazeFilter
{
public:
    bool operator()(WorldObject* unit)
    {
        if (Player* player = unit->ToPlayer())
            if (player->HasAura(SPELL_LINGERING_GAZE_TARGET))
                return false;
        return true;
    }
};

class boss_durumu : public CreatureScript
{
public:
    boss_durumu() : CreatureScript("boss_durumu") {}

    struct boss_durumuAI : public BossAI
    {
        boss_durumuAI(Creature* creature) : BossAI(creature, DATA_DURUMU)
        {
            instance = creature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            DoCast(me, SPELL_DURUMU_SPAWN, true);
        }
        InstanceScript* instance;
        Phase phase;
        uint32 checkvictim;
        ObjectGuid eyebeamtargetGuid;
        uint32 enragetimer;
        bool rotatedirection;
        GuidVector crosseyelist;
        uint8 lastmiststage;
        uint8 nextmiststage;
        uint8 way;   //first safe line in mist
        uint8 way2;  //second safe line on mist

        void Reset()
        {
            _Reset();
            phase = PHASE_NULL;
            checkvictim = 0;
            enragetimer = 0;
            eyebeamtargetGuid.Clear();
            rotatedirection = false;
            crosseyelist.clear();
            RemoveDebuffFromPlayers();
            me->SetReactState(REACT_DEFENSIVE);
            me->RemoveAurasDueToSpell(SPELL_MIND_DAGGERS_AURA);
            me->RemoveAurasDueToSpell(SPELL_BRIGHT_LIGHT_DURUMU);
            me->RemoveAurasDueToSpell(SPELL_DISINTEGRATION_LASER_AT);
            instance->SetData(DATA_CLEAR_CRIMSON_FOG_LIST, 0);
            instance->DoUpdateWorldState(WorldStates::WORLD_STATE_ALIVE_FOG_COUNT, 0);
        }

        void KilledUnit(Unit* unit)
        {
            if (unit->ToPlayer())
                Talk(SAY_KILL_PLAYER);
        }

        uint32 GetData(uint32 type) const
        {
            if (type == DATA_GET_DURUMU_ROTATE_DIRECTION)
                return uint32(rotatedirection);
            return 0;
        }

        float GetFogAngle(uint8 pos)
        {
            switch (pos)
            {
            case 0:
            {
                float mod = float(urand(0, 1));
                float mod2 = !mod ? float(urand(0, 9)) / 10 : float(urand(0, 3)) / 10;
                return mod + mod2;
            }
            case 1:
            {
                float mod = float(urand(1, 2));
                float mod2 = mod == 1 ? float(urand(6, 9)) / 10 : float(urand(0, 9)) / 10;
                return mod + mod2;
            }
            case 2:
            {
                float mod = float(urand(3, 4));
                float mod2 = mod == 3 ? float(urand(2, 9)) / 10 : float(urand(0, 3)) / 10;
                return mod + mod2;
            }
            case 3:
            {
                float mod = float(urand(4, 5));
                float mod2 = mod == 4 ? float(urand(6, 9)) / 10 : float(urand(0, 9)) / 10;
                return mod + mod2;
            }
            default:
                return 0;
            }
        }

        void SummonFogs()
        {
            float x, y;
            float ang = 0;
            uint32 summonspellentry = 0;
            uint8 bluefog = urand(0, 3);
            for (uint8 n = 0; n < 4; n++)
            {
                ang = GetFogAngle(n);
                summonspellentry = bluefog == n ? SPELL_SUMMON_BLUE_FOG : SPELL_SUMMON_RED_FOG;
                me->GetNearPoint2D(x, y, 12.0f, ang);
                me->CastSpell(x, y, -6.27f, summonspellentry, true);
            }
        }

        void RemoveDebuffFromPlayers()
        {
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_INFRARED_LIGHT_BEAM);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLUE_RAY_BEAM);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BRIGHT_LIGHT_BEAM);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_INFRARED_LIGHT_CONE_DMG);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLUE_RAY_CONE_DMG);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BRIGHT_LIGHT_CONE_DMG);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_INFRARED_LIGHT_P_T_AURA);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLUE_RAY_P_T_AURA);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BRIGHT_LIGHT_P_T_AURA);
        }

        void EnterCombat(Unit* who)
        {
            _EnterCombat();
            phase = PHASE_NORMAL;
            Talk(SAY_ENTERCOMBAT);
            checkvictim = 3000;
            enragetimer = 600000;
            events.RescheduleEvent(EVENT_HARD_STARE, 12000);
            events.RescheduleEvent(EVENT_LINGERING_GAZE_PREPARE, 13000);
            events.RescheduleEvent(EVENT_FORCE_OF_WILL_PREPARE, 30000);
            events.RescheduleEvent(EVENT_COLORBLIND, 42000);

            //test events
            //events.RescheduleEvent(EVENT_DISINTEGRATION_BEAM_P, 5000);
        }

        void SetData(uint32 type, uint32 data)
        {
            uint32 explosespell = 0;
            switch (type)
            {
            case SPELL_INFRARED_LIGHT_CONE_DMG:
                explosespell = SPELL_INFRARED_LIGHT_EXPLOSE;
                break;
            case SPELL_BLUE_RAY_CONE_DMG:
                explosespell = SPELL_BLUE_RAY_EXPLOSE;
                break;
            case SPELL_BRIGHT_LIGHT_CONE_DMG:
                explosespell = SPELL_BRIGHT_LIGHT_EXPLOSE;
                break;
            }
            DoCast(me, explosespell, true);
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_COLORBLIND_PHASE_DONE:
                summons.DespawnEntry(NPC_RED_EYE);
                summons.DespawnEntry(NPC_BLUE_EYE);
                summons.DespawnEntry(NPC_YELLOW_EYE);
                summons.DespawnEntry(NPC_RED_EYEBEAM_TARGET);
                summons.DespawnEntry(NPC_BLUE_EYEBEAM_TARGET);
                summons.DespawnEntry(NPC_YELLOW_EYEBEAM_TARGET);
                summons.DespawnEntry(NPC_CRIMSON_FOG);
                summons.DespawnEntry(NPC_AZURE_FOG);
                RemoveDebuffFromPlayers();
                phase = PHASE_NORMAL;
                me->RemoveAurasDueToSpell(SPELL_BRIGHT_LIGHT_DURUMU);
                events.RescheduleEvent(EVENT_HARD_STARE, 12000);
                events.RescheduleEvent(EVENT_LINGERING_GAZE, 4000);
                events.RescheduleEvent(EVENT_FORCE_OF_WILL, 8000);
                events.RescheduleEvent(EVENT_COLORBLIND, 65000);
                //events.RescheduleEvent(EVENT_DISINTEGRATION_BEAM, 65000); not ready
                break;
            case ACTION_LAUNCH_ROTATE:
                lastmiststage = 0;
                nextmiststage = 1;
                way = urand(3, 6);
                way2 = urand(8, 10);
                events.RescheduleEvent(EVENT_OPEN_WAY_IN_MIST, 2000);
                break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            Talk(SAY_DIE);
            RemoveDebuffFromPlayers();
            instance->DoUpdateWorldState(WorldStates::WORLD_STATE_ALIVE_FOG_COUNT, 0);
            _JustDied();
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (checkvictim)
            {
                if (checkvictim <= diff)
                {
                    if (me->getVictim() && !me->IsWithinMeleeRange(me->getVictim()))
                    {
                        if (me->GetDistance(me->getVictim()) < 60.0f)
                            DoCastAOE(SPELL_GAZE);
                        else
                            EnterEvadeMode();
                    }
                    checkvictim = 3000;
                }
                else
                    checkvictim -= diff;
            }

            if (enragetimer)
            {
                if (enragetimer <= diff)
                {
                    enragetimer = 0;
                    DoCastAOE(SPELL_OBLITERATE);
                }
                else
                    enragetimer -= diff;
            }

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING) && phase != PHASE_DISINTEGRATION_BEAM)
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                //Normal and Advance phase
                case EVENT_HARD_STARE:
                    if (me->getVictim())
                        DoCast(me->getVictim(), SPELL_HARD_STARE);
                    events.RescheduleEvent(EVENT_HARD_STARE, 12000);
                    break;
                case EVENT_FORCE_OF_WILL_PREPARE:
                {
                    float x, y;
                    float mod = urand(0, 6);
                    float ang = mod <= 5 ? mod + float(urand(1, 9)) / 10 : mod;
                    me->GetNearPoint2D(x, y, -15.0f, ang);
                    me->SummonCreature(NPC_MIND_EYE, x, y, -2.0f, 0.0f);
                    events.RescheduleEvent(EVENT_FORCE_OF_WILL, 5000);
                    break;
                }
                case EVENT_FORCE_OF_WILL:
                    Talk(SAY_FORCE_OF_WILL);
                    if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 80.0f, true))
                        if (Creature* mindeye = me->FindNearestCreature(NPC_MIND_EYE, 50.0f, true))
                            mindeye->AI()->SetGUID(target->GetGUID(), 1);
                    events.RescheduleEvent(EVENT_FORCE_OF_WILL, 18000);
                    break;
                case EVENT_LINGERING_GAZE_PREPARE:
                {
                    float x, y;
                    float mod = urand(0, 6);
                    float ang = mod <= 5 ? mod + float(urand(1, 9)) / 10 : mod;
                    me->GetNearPoint2D(x, y, -15.0f, ang);
                    me->SummonCreature(NPC_APPRAISING_EYE, x, y, 2.46f, 0.0f);
                    events.RescheduleEvent(EVENT_LINGERING_GAZE, 5000);
                    break;
                }
                case EVENT_LINGERING_GAZE:
                {
                    uint8 maxnum = me->GetMap()->Is25ManRaid() ? 5 : 2;
                    uint8 num = 0;
                    std::list<Player*>pllist;
                    pllist.clear();
                    GetPlayerListInGrid(pllist, me, 100.0f);
                    if (!pllist.empty())
                    {
                        pllist.remove_if(_TankFilter());
                        if (!pllist.empty())
                        {
                            for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                            {
                                DoCast(*itr, SPELL_LINGERING_GAZE_TARGET, true);
                                num++;
                                if (num >= maxnum)
                                    break;
                            }
                        }
                    }
                    if (Creature* appraisingeye = me->FindNearestCreature(NPC_APPRAISING_EYE, 50.0f, true))
                        appraisingeye->AI()->DoAction(ACTION_LINGERING_GAZE);
                    events.RescheduleEvent(EVENT_LINGERING_GAZE, 35000);
                    break;
                }
                case EVENT_LIFE_DRAIN_PREPARE:
                {
                    Creature* hungryeye = me->FindNearestCreature(NPC_HUNGRY_EYE, 100.0f, true);
                    if (!hungryeye)
                    {
                        float x, y;
                        float mod = urand(0, 6);
                        float ang = mod <= 5 ? mod + float(urand(1, 9)) / 10 : mod;
                        me->GetNearPoint2D(x, y, -15.0f, ang);
                        me->SummonCreature(NPC_HUNGRY_EYE, x, y, 1.0f, 0.0f);
                    }
                    events.RescheduleEvent(EVENT_LIFE_DRAIN, 5000);
                    break;
                }
                case EVENT_LIFE_DRAIN:
                    if (Creature* hungryeye = me->FindNearestCreature(NPC_HUNGRY_EYE, 50.0f, true))
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            hungryeye->AI()->SetGUID(target->GetGUID(), 1);
                    break;
                //Colorblind phase
                case EVENT_COLORBLIND:
                {
                    events.CancelEvent(EVENT_FORCE_OF_WILL);
                    me->InterruptNonMeleeSpells(true);
                    phase = PHASE_COLORBLIND;
                    Talk(SAY_COLORBLIND);
                    std::list<Player*> pllist;
                    GetPlayerListInGrid(pllist, me, 150.0f);
                    if (!pllist.empty())
                    {
                        if (pllist.size() == 1) //WOD or Legion solo kill
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                                for (uint8 n = 0; n < 3; n++)
                                    if (Creature* colorblindeye = me->SummonCreature(colorblindeyelist[n], Durumucenterpos))
                                        colorblindeye->AI()->SetGUID(target->GetGUID(), 1);
                        }
                        else if (pllist.size() == 2)
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 150.0f, true))
                                for (uint8 n = 0; n < 3; n++)
                                    if (Creature* colorblindeye = me->SummonCreature(colorblindeyelist[n], Durumucenterpos))
                                        colorblindeye->AI()->SetGUID(target->GetGUID(), 1);
                        }
                        else if (pllist.size() == 3)
                        {
                            Trinity::Containers::RandomResizeList(pllist, 1);
                            for (uint8 n = 0; n < 3; n++)
                                if (Creature* colorblindeye = me->SummonCreature(colorblindeyelist[n], Durumucenterpos))
                                    for (auto const& player : pllist)
                                        colorblindeye->AI()->SetGUID(player->GetGUID(), 1);
                        }
                        else if (pllist.size() >= 4)
                        {
                            uint8 index = 0;
                            ObjectGuid victimGuid = me->getVictim() ? me->getVictim()->GetGUID() : ObjectGuid::Empty;
                            pllist.remove_if(VictimFilter(victimGuid));
                            for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                            {
                                if (Creature* colorblindeye = me->SummonCreature(colorblindeyelist[index], Durumucenterpos))
                                {
                                    index++;
                                    colorblindeye->AI()->SetGUID((*itr)->GetGUID(), 1);
                                    if (index >= 3)
                                        break;
                                }
                            }
                        }
                    }
                    instance->DoUpdateWorldState(WorldStates::WORLD_STATE_ALIVE_FOG_COUNT, 1);
                    instance->DoUpdateWorldState(WorldStates::WORLD_STATE_ALIVE_FOG_COUNT, 3);
                    SummonFogs();
                    break;
                }
                //Disintegration beam phase
                case EVENT_DISINTEGRATION_BEAM_P:
                    phase = PHASE_DISINTEGRATION_BEAM;
                    events.Reset();
                    me->InterruptNonMeleeSpells(true);
                    me->StopAttack();
                    Talk(SAY_DISINTEGRATION_START);
                    if (Creature* eyebeamtarget = me->SummonCreature(NPC_EYEBEAM_TARGET_DURUMU, Eyebeamtargetpos, TEMPSUMMON_TIMED_DESPAWN, 54000))
                    {
                        me->SetFacingToObject(eyebeamtarget);
                        eyebeamtargetGuid = eyebeamtarget->GetGUID();
                    }
                    crosseyelist.clear();
                    events.RescheduleEvent(EVENT_DISINTEGRATION_BEAM, 250);
                    events.RescheduleEvent(EVENT_ADVANCE_PHASE, 55000);
                    break;
                case EVENT_DISINTEGRATION_BEAM:
                    if (Creature* eyebeamtarget = me->GetCreature(*me, eyebeamtargetGuid))
                        DoCast(eyebeamtarget, SPELL_DISINTEGRATION_LASER_P);
                    DoCast(me, SPELL_MIND_DAGGERS_AURA);
                    rotatedirection = bool(urand(0, 1));
                    events.RescheduleEvent(EVENT_PREPARE_TO_MIST, 6000);
                    break;
                case EVENT_PREPARE_TO_MIST:
                {
                    switch (rotatedirection)
                    {
                    case 0:       //left
                        for (uint8 b = 11; b > 8; b--)
                        {
                            if (Creature* lce = me->SummonCreature(NPC_CROSS_EYE, triggerspawnpos[b]))
                            {
                                crosseyelist.push_back(lce->GetGUID());
                                lce->CastSpell(lce, SPELL_MAZE_COLLECTED);
                            }
                        }
                        break;
                    case 1:       //right
                        for (uint8 n = 0; n < 3; n++)
                        {
                            if (Creature* rce = me->SummonCreature(NPC_CROSS_EYE, triggerspawnpos[n]))
                            {
                                crosseyelist.push_back(rce->GetGUID());
                                rce->CastSpell(rce, SPELL_MAZE_COLLECTED);
                            }
                        }
                        break;
                    }
                    events.RescheduleEvent(EVENT_CREATE_MIST, 2000);
                    break;
                }
                case EVENT_CREATE_MIST:
                {
                    std::list<Creature*>swaplist;
                    swaplist.clear();

                    switch (rotatedirection)
                    {
                    case 0:
                        for (uint8 b = 0; b < 9; b++)
                        {
                            if (Creature* lce = me->SummonCreature(NPC_CROSS_EYE, triggerspawnpos[b]))
                            {
                                swaplist.push_back(lce);
                                lce->AI()->SetData(DATA_CREATE_MIST, 1000 + b * 650);
                            }
                        }
                        if (!swaplist.empty())
                        {
                            swaplist.reverse();
                            for (std::list<Creature*>::const_iterator itr = swaplist.begin(); itr != swaplist.end(); itr++)
                                crosseyelist.push_back((*itr)->GetGUID());
                        }
                        break;
                    case 1:
                        for (uint8 n = 11; n > 2; n--)
                        {
                            if (Creature* rce = me->SummonCreature(NPC_CROSS_EYE, triggerspawnpos[n]))
                            {
                                swaplist.push_back(rce);
                                rce->AI()->SetData(DATA_CREATE_MIST, 1000 + (11 - n) * 650);
                            }
                        }
                        if (!swaplist.empty())
                        {
                            swaplist.reverse();
                            for (std::list<Creature*>::const_iterator itr = swaplist.begin(); itr != swaplist.end(); itr++)
                                crosseyelist.push_back((*itr)->GetGUID());
                        }
                        break;
                    }
                    break;
                }
                case EVENT_OPEN_WAY_IN_MIST:
                {
                    if (nextmiststage > 2 && nextmiststage <= 11)
                    {
                        if (Creature* nextcrosseye = me->GetCreature(*me, crosseyelist[nextmiststage]))
                        {
                            if (AreaTrigger* at = nextcrosseye->GetAreaObject(SPELL_TKR_WHOLE_SLICE_X_1))
                                at->RemoveFromWorld();

                            for (uint8 n = 0; n < 11; n++)
                                if (n != way && n != way2)
                                    nextcrosseye->CastSpell(nextcrosseye, wholeroomslice[n]);
                        }
                    }

                    if (nextmiststage > 11)
                        return;

                    uint32 timer = 0; //timer - open way in mist
                    uint32 data = 0;  //timer - close safe zone
                    switch (rotatedirection)
                    {
                    case 0:      //left
                        timer = nextmiststage <= 3 ? 2000 : 4000;
                        data = nextmiststage <= 3 ? 4000 : 7000;
                        break;
                    case 1:      //right
                        timer = nextmiststage <= 3 ? 1000 : 4000;
                        data = nextmiststage < 3 ? 2500 : 5500;
                        break;
                    }
                    events.RescheduleEvent(EVENT_OPEN_WAY_IN_MIST, timer);
                    if (Creature* lastcrosseye = me->GetCreature(*me, crosseyelist[lastmiststage]))
                        lastcrosseye->AI()->SetData(DATA_CLOSE_SAFE_ZONE_IN_MIST, data);
                    nextmiststage++;
                    lastmiststage++;
                    break;
                }
                case EVENT_ADVANCE_PHASE:
                    phase = PHASE_ADVANCE;
                    me->SetReactState(REACT_AGGRESSIVE);
                    summons.DespawnEntry(NPC_CROSS_EYE);
                    events.RescheduleEvent(EVENT_HARD_STARE, 12000);
                    events.RescheduleEvent(EVENT_LIFE_DRAIN_PREPARE, 13000);
                    events.RescheduleEvent(EVENT_LINGERING_GAZE, 15000);
                    events.RescheduleEvent(EVENT_FORCE_OF_WILL, 35000);
                    events.RescheduleEvent(EVENT_COLORBLIND, 42000);
                    break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_durumuAI(creature);
    }
};

//67858
class npc_appraising_eye : public CreatureScript
{
public:
    npc_appraising_eye() : CreatureScript("npc_appraising_eye") {}

    struct npc_appraising_eyeAI : public ScriptedAI
    {
        npc_appraising_eyeAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
        }
        InstanceScript* pInstance;
        EventMap events;
        uint8 direction;

        void Reset()
        {
            direction = urand(0, 1);
            DoZoneInCombat(me, 100.0f);
            DoCast(me, SPELL_DURUMU_EYE_TRAIL_ORANGE, true);
            events.RescheduleEvent(EVENT_START_MOVE, 250);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_LINGERING_GAZE)
            {
                events.Reset();
                me->GetMotionMaster()->Clear(false);
                DoCast(me, SPELL_LINGERING_GAZE);
                events.RescheduleEvent(EVENT_START_MOVE, 4000);
            }
        }

        float GetNewAngle()
        {
            if (Creature* durumu = me->GetCreature(*me, pInstance->GetGuidData(NPC_DURUMU)))
            {
                if (direction)
                    return durumu->GetAngle(me) + 0.5f;
                else
                    return durumu->GetAngle(me) - 0.5f;
            }
            return 0;
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_START_MOVE)
                {
                    if (Creature* durumu = me->GetCreature(*me, pInstance->GetGuidData(NPC_DURUMU)))
                    {
                        float x, y;
                        float ang = GetNewAngle();
                        durumu->GetNearPoint2D(x, y, -15.0f, ang);
                        me->GetMotionMaster()->Clear(false);
                        me->GetMotionMaster()->MoveCharge(x, y, me->GetPositionZ(), 10.0f, 1);
                        events.RescheduleEvent(EVENT_START_MOVE, 250);
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_appraising_eyeAI(creature);
    }
};

//67875
class npc_mind_eye : public CreatureScript
{
public:
    npc_mind_eye() : CreatureScript("npc_mind_eye") {}

    struct npc_mind_eyeAI : public ScriptedAI
    {
        npc_mind_eyeAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetCanFly(true);
            me->SetDisableGravity(true);
        }
        InstanceScript* pInstance;
        EventMap events;
        uint8 direction;

        void Reset()
        {
            direction = urand(0, 1);
            DoZoneInCombat(me, 100.0f);
            DoCast(me, SPELL_DURUMU_EYE_TRAIL_BLUE, true);
            events.RescheduleEvent(EVENT_START_MOVE, 250);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void SetGUID(ObjectGuid const& guid, int32 /*type*/) override
        {
            events.Reset();
            me->GetMotionMaster()->Clear(false);
            if (Player* pl = me->GetPlayer(*me, guid))
            {
                me->SetFacingToObject(pl);
                me->CastSpell(pl, SPELL_FORCE_OF_WILL_DUMMY);
            }
            events.RescheduleEvent(EVENT_START_MOVE, 4000);
        }

        float GetNewAngle()
        {
            if (Creature* durumu = me->GetCreature(*me, pInstance->GetGuidData(NPC_DURUMU)))
            {
                if (direction)
                    return durumu->GetAngle(me) + 0.5f;
                else
                    return durumu->GetAngle(me) - 0.5f;
            }
            return 0;
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_START_MOVE)
                {
                    if (Creature* durumu = me->GetCreature(*me, pInstance->GetGuidData(NPC_DURUMU)))
                    {
                        float x, y;
                        float ang = GetNewAngle();
                        durumu->GetNearPoint2D(x, y, -15.0f, ang);
                        me->GetMotionMaster()->Clear(false);
                        me->GetMotionMaster()->MoveCharge(x, y, me->GetPositionZ(), 10.0f, 1);
                    }
                    events.RescheduleEvent(EVENT_START_MOVE, 250);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mind_eyeAI(creature);
    }
};

//67859
class npc_hungry_eye : public CreatureScript
{
public:
    npc_hungry_eye() : CreatureScript("npc_hungry_eye") {}

    struct npc_hungry_eyeAI : public ScriptedAI
    {
        npc_hungry_eyeAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetCanFly(true);
            me->SetDisableGravity(true);
        }
        InstanceScript* pInstance;
        EventMap events;
        ObjectGuid targetGuid;
        uint8 direction;

        void Reset()
        {
            targetGuid.Clear();
            direction = urand(0, 1);
            DoZoneInCombat(me, 100.0f);
            DoCast(me, SPELL_DURUMU_EYE_TRAIL_YELLOW, true);
            events.RescheduleEvent(EVENT_START_MOVE, 250);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void SetGUID(ObjectGuid const& guid, int32 /*type*/) override
        {
            events.Reset();
            me->GetMotionMaster()->Clear(false);
            if (Player* pl = me->GetPlayer(*me, guid))
            {
                targetGuid = guid;
                me->SetFacingToObject(pl);
                DoCast(pl, SPELL_LIFE_DRAIN_WARNING, true);
                events.RescheduleEvent(EVENT_LIFE_DRAIN_LAUNCH, 100);
            }
        }

        float GetNewAngle()
        {
            if (Creature* durumu = me->GetCreature(*me, pInstance->GetGuidData(NPC_DURUMU)))
            {
                if (direction)
                    return durumu->GetAngle(me) + 0.5f;
                else
                    return durumu->GetAngle(me) - 0.5f;
            }
            return 0.0f;
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_START_MOVE)
                {
                    if (Creature* durumu = me->GetCreature(*me, pInstance->GetGuidData(NPC_DURUMU)))
                    {
                        float x, y;
                        float ang = GetNewAngle();
                        durumu->GetNearPoint2D(x, y, -15.0f, ang);
                        me->GetMotionMaster()->Clear(false);
                        me->GetMotionMaster()->MoveCharge(x, y, me->GetPositionZ(), 10.0f, 1);
                    }
                    events.RescheduleEvent(EVENT_START_MOVE, 250);
                }
                else if (eventId == EVENT_LIFE_DRAIN_LAUNCH)
                {
                    if (Player* pl = me->GetPlayer(*me, targetGuid))
                    {
                        DoCast(pl, SPELL_LIFE_DRAIN_DUMMY, true);
                        pl->AddAura(SPELL_LIFE_DRAIN_STUN, pl);
                    }
                    events.RescheduleEvent(EVENT_START_MOVE, 16000);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_hungry_eyeAI(creature);
    }
};

//67855, 67854, 67856
class npc_colorblind_eye : public CreatureScript
{
public:
    npc_colorblind_eye() : CreatureScript("npc_colorblind_eye") { }

    struct npc_colorblind_eyeAI : public ScriptedAI
    {
        npc_colorblind_eyeAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(11686);
        }

        InstanceScript* instance;
        EventMap events;
        ObjectGuid targetGuid;

        void Reset(){}

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_CREATE_CONE)
                events.RescheduleEvent(EVENT_PREPARE_BEAM, 100);
        }

        uint32 GetEyeBeamTargetEntry()
        {
            switch (me->GetEntry())
            {
            case NPC_RED_EYE:
                return NPC_RED_EYEBEAM_TARGET;
            case NPC_BLUE_EYE:
                return NPC_BLUE_EYEBEAM_TARGET;
            default:
                return 0;
            }
        }

        uint32 GetBeamSpellEntry()
        {
            switch (me->GetEntry())
            {
            case NPC_RED_EYE:
                return SPELL_INFRARED_LIGHT_BEAM;
            case NPC_BLUE_EYE:
                return SPELL_BLUE_RAY_BEAM;
            case NPC_YELLOW_EYE:
                return SPELL_BRIGHT_LIGHT_BEAM;
            default:
                return 0;
            }
        }

        uint32 GetConeSpellEntry()
        {
            switch (me->GetEntry())
            {
            case NPC_RED_EYE:
                return SPELL_INFRARED_LIGHT_CONE;
            case NPC_BLUE_EYE:
                return SPELL_BLUE_RAY_CONE;
            case NPC_YELLOW_EYE:
                return SPELL_BRIGHT_LIGHT_CONE;
            default:
                return 0;
            }
        }

        uint32 GetLightPlayerAura()
        {
            switch (me->GetEntry())
            {
            case NPC_RED_EYE:
                return SPELL_INFRARED_LIGHT_P_T_AURA;
            case NPC_BLUE_EYE:
                return SPELL_BLUE_RAY_P_T_AURA;
            case NPC_YELLOW_EYE:
                return SPELL_BRIGHT_LIGHT_P_T_AURA;
            default:
                return 0;
            }
        }

        uint32 GetLightFogAura()
        {
            switch (me->GetEntry())
            {
            case NPC_RED_EYE:
                return SPELL_INFRARED_LIGHT_C_T_AURA;
            case NPC_BLUE_EYE:
                return SPELL_BLUE_RAY_C_T_AURA;
            case NPC_YELLOW_EYE:
                return 0;
            default:
                return 0;
            }
        }

        uint32 GetSearcheFogEntry()
        {
            switch (me->GetEntry())
            {
            case NPC_RED_EYE:
                return NPC_CRIMSON_FOG;
            case NPC_BLUE_EYE:
                return NPC_AZURE_FOG;
            case NPC_YELLOW_EYE:
                return 0;
            default:
                return 0;
            }
        }

        uint32 GetData(uint32 type) const
        {
            if (type == DATA_IS_CONE_TARGET_CREATURE)
            {
                if (Unit* conetarget = me->GetUnit(*me, targetGuid))
                {
                    if (conetarget->ToCreature())
                        return 1;
                    else
                        return 0;
                }
            }
            return 0;
        }

        void SetData(uint32 type, uint32 data)
        {
            //remove focus cone from trigger, and despawn him
            if (type == DATA_DESPAWN_CREATURE_CONE_TARGET)
            {
                if (Creature* conetarget = me->GetCreature(*me, targetGuid))
                {
                    uint32 conespell = GetConeSpellEntry();
                    conetarget->RemoveAurasDueToSpell(conespell);//for safe
                    conetarget->DespawnOrUnsummon();
                }
            }
        }

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            targetGuid = guid;
            switch (id)
            {
            case 1:
            {
                uint32 beamspell = GetBeamSpellEntry();
                if (Player* pl = me->GetPlayer(*me, targetGuid))
                    DoCast(pl, beamspell, true);
                break;
            }
            //focus cone player died, create cone with trigger
            case 2:
                if (Creature* durumu = me->GetCreature(*me, instance->GetGuidData(NPC_DURUMU)))
                {
                    if (Player* pl = me->GetPlayer(*me, targetGuid))
                    {
                        Position pos;
                        pl->GetPosition(&pos);
                        uint32 conespell = GetConeSpellEntry();
                        uint32 npceyebeamtarget = GetEyeBeamTargetEntry();
                        if (Creature* conetarget = durumu->SummonCreature(npceyebeamtarget, pos))
                        {
                            targetGuid = conetarget->GetGUID();
                            DoCast(conetarget, conespell, true);
                        }
                    }
                }
                break;
            //create focus cone on new player
            case 3:
            {
                uint32 conespell = GetConeSpellEntry();
                if (Player* pl = me->GetPlayer(*me, targetGuid))
                    DoCast(pl, conespell, true);
            }
            break;
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_PREPARE_BEAM: //this event create for fix rotate lag, before launch cone spell
                    if (Player* pl = me->GetPlayer(*me, targetGuid))
                    {
                        me->AddThreat(pl, 50000000.0f);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->Attack(pl, true);
                    }
                    events.RescheduleEvent(EVENT_CREATE_CONE, 250);
                    break;
                case EVENT_CREATE_CONE:
                    if (Player* pl = me->GetPlayer(*me, targetGuid))
                    {
                        uint32 conespell = GetConeSpellEntry();
                        if (Player* pl = me->GetPlayer(*me, targetGuid))
                        {
                            if (me->GetEntry() == NPC_YELLOW_EYE)
                            {
                                float x, y;
                                float ang = me->GetAngle(pl);
                                me->GetNearPoint2D(x, y, 64.0f, ang);
                                if (Creature* durumu = me->GetCreature(*me, instance->GetGuidData(NPC_DURUMU)))
                                {
                                    if (Creature* conetarget = durumu->SummonCreature(NPC_YELLOW_EYEBEAM_TARGET, x, y, me->GetPositionZ(), 0.0f))
                                    {
                                        durumu->CastSpell(durumu, SPELL_BRIGHT_LIGHT_DURUMU, true); //visual
                                        DoCast(conetarget, conespell, true);
                                        conetarget->AI()->SetGUID(me->GetGUID(), 2);
                                    }
                                }
                            }
                            else
                                DoCast(pl, conespell, true);
                        }
                        events.RescheduleEvent(EVENT_SEARCHER, 1000);
                    }
                    break;
                case EVENT_SEARCHER:
                {
                    //Search players in cone
                    uint32 lightaura = GetLightPlayerAura();
                    std::list<Player*>pllist;
                    GetPlayerListInGrid(pllist, me, 100.0f);
                    if (!pllist.empty())
                    {
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                        {
                            if (me->isInFront(*itr, M_PI / 6))
                            {
                                if (!(*itr)->HasAura(lightaura))
                                    (*itr)->CastSpell(*itr, lightaura, true);
                            }
                            else
                                if ((*itr)->HasAura(lightaura))
                                    (*itr)->RemoveAurasDueToSpell(lightaura);
                        }
                    }
                    //Search fogs in cone
                    if (me->GetEntry() != NPC_YELLOW_EYE)
                    {
                        uint32 _lightaura = GetLightFogAura();
                        uint32 fogentry = GetSearcheFogEntry();
                        std::list<Creature*>foglist;
                        GetCreatureListWithEntryInGrid(foglist, me, fogentry, 150.0f);
                        if (!foglist.empty())
                        {
                            for (std::list<Creature*>::const_iterator Itr = foglist.begin(); Itr != foglist.end(); Itr++)
                            {
                                if (me->isInFront(*Itr, M_PI / 6))
                                {
                                    if (!(*Itr)->HasAura(_lightaura))
                                        (*Itr)->CastSpell(*Itr, _lightaura, true);
                                }
                                else
                                    if ((*Itr)->HasAura(_lightaura))
                                        (*Itr)->RemoveAurasDueToSpell(_lightaura);
                            }
                        }
                    }
                    events.RescheduleEvent(EVENT_SEARCHER, 1000);
                }
                break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_colorblind_eyeAI(pCreature);
    }
};

//67851, 67829, 67852
class npc_colorblind_eye_beam_target : public CreatureScript
{
public:
    npc_colorblind_eye_beam_target() : CreatureScript("npc_colorblind_eye_beam_target") { }

    struct npc_colorblind_eye_beam_targetAI : public ScriptedAI
    {
        npc_colorblind_eye_beam_targetAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(11686);
        }
        EventMap events;
        ObjectGuid colorblindeyeGuid;

        void Reset(){}

        void SetGUID(ObjectGuid const& guid, int32 id) override
        {
            if (id == 2)
            {
                colorblindeyeGuid = guid;
                events.RescheduleEvent(EVENT_START_MOVE, 1000);
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_START_MOVE)
                {
                    if (Creature* colorblindeye = me->GetCreature(*me, colorblindeyeGuid))
                    {
                        float x, y;
                        float ang = colorblindeye->GetAngle(me) - 0.5f;
                        colorblindeye->GetNearPoint2D(x, y, 64.0f, ang);
                        me->GetMotionMaster()->Clear(false);
                        me->GetMotionMaster()->MoveCharge(x, y, me->GetPositionZ(), 5.0f, 1);
                    }
                    events.RescheduleEvent(EVENT_START_MOVE, 250);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_colorblind_eye_beam_targetAI(pCreature);
    }
};

//69050, 69052
class npc_durumu_fog : public CreatureScript
{
public:
    npc_durumu_fog() : CreatureScript("npc_durumu_fog") {}

    struct npc_durumu_fogAI : public ScriptedAI
    {
        npc_durumu_fogAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            explose = false;
            switch (me->GetEntry())
            {
                case NPC_CRIMSON_FOG:
                    DoCast(me, SPELL_RED_FOG_INVISIBILITY, true);
                    break;
                case NPC_AZURE_FOG:
                    DoCast(me, SPELL_BLUE_FOG_INVISIBILITY, true);
                    break;
                default:
                    break;
            }
        }
        InstanceScript* instance;
        EventMap events;
        uint32 invisibilyaura;
        uint32 foundspell;
        bool explose;

        void Reset()
        {
            switch (me->GetEntry())
            {
            case NPC_CRIMSON_FOG:
                invisibilyaura = SPELL_RED_FOG_INVISIBILITY;
                foundspell = SPELL_BURNING_EYE_FOUND;
                break;
            case NPC_AZURE_FOG:
                invisibilyaura = SPELL_BLUE_FOG_INVISIBILITY;
                foundspell = SPELL_COLD_EYE_FOUND;
                break;
            default:
                invisibilyaura = 0;
                foundspell = 0;
                break;
            }
        }

        void DoAction(int32 const action)
        {
            switch (action)
            {
            case ACTION_IN_CONE:
                me->RemoveAurasDueToSpell(invisibilyaura);
                DoCast(me, foundspell, true);
                if (me->GetEntry() == NPC_CRIMSON_FOG)
                    events.RescheduleEvent(EVENT_CAUSTIC_SPIKE, 2000);
                else if (me->GetEntry() == NPC_AZURE_FOG)
                    DoCast(me, SPELL_ICY_GRASP_AURA, true);
                break;
            case ACTION_NOT_IN_CONE:
                events.Reset();
                me->InterruptNonMeleeSpells(true);
                if (me->GetEntry() == NPC_CRIMSON_FOG)
                    DoCast(me, SPELL_CRIMSON_BLOOM, true);
                me->AddAura(invisibilyaura, me);
                break;
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (me->GetEntry() != NPC_AZURE_FOG)
                return;

            if (damage >= me->GetHealth() && !explose)
            {
                explose = true;
                DoCast(me, SPELL_FLASH_FREEZE, true);
            }
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                if (eventId == EVENT_CAUSTIC_SPIKE)
                {
                    DoCastAOE(SPELL_CAUSTIC_SPIKE);
                    events.RescheduleEvent(EVENT_CAUSTIC_SPIKE, 3000);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_durumu_fogAI(creature);
    }
};

//67882
class npc_durumu_eyebeam_target : public CreatureScript
{
public:
    npc_durumu_eyebeam_target() : CreatureScript("npc_durumu_eyebeam_target") {}

    struct npc_durumu_eyebeam_targetAI : public ScriptedAI
    {
        npc_durumu_eyebeam_targetAI(Creature* creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->SetDisplayId(11686);
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }
        InstanceScript* pInstance;
        EventMap events;
        bool rotatedirection;
        float disttodurumu;

        void Reset()
        {
            if (Creature* durumu = me->GetCreature(*me, pInstance->GetGuidData(NPC_DURUMU)))
                disttodurumu = me->GetExactDist2d(durumu) - 20.0f;
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_LAUNCH_ROTATE)
            {
                if (Creature* durumu = me->GetCreature(*me, pInstance->GetGuidData(NPC_DURUMU)))
                {
                    durumu->AI()->DoAction(ACTION_LAUNCH_ROTATE);
                    rotatedirection = durumu->AI()->GetData(DATA_GET_DURUMU_ROTATE_DIRECTION);
                    events.RescheduleEvent(EVENT_START_MOVE, 100);
                }
            }
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
                if (eventId == EVENT_START_MOVE)
                {
                    if (Creature* durumu = me->GetCreature(*me, pInstance->GetGuidData(NPC_DURUMU)))
                    {
                        float x, y;
                        float ang = 0;
                        if (!rotatedirection)
                            ang = durumu->GetAngle(me) - 0.5f; //left
                        else
                            ang = durumu->GetAngle(me) + 0.5f; //right
                        durumu->GetNearPoint2D(x, y, disttodurumu, ang);
                        me->GetMotionMaster()->Clear(false);
                        me->GetMotionMaster()->MoveJump(x, y, -2.433161f, 10.0f, 0.0f, 1);
                    }
                    events.RescheduleEvent(EVENT_START_MOVE, 250);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_durumu_eyebeam_targetAI(creature);
    }
};

//67857
class npc_cross_eye : public CreatureScript
{
public:
    npc_cross_eye() : CreatureScript("npc_cross_eye") { }

    struct npc_cross_eyeAI : public ScriptedAI
    {
        npc_cross_eyeAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            instance = pCreature->GetInstanceScript();
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(11686);
            rotatedirection = false;
        }
        InstanceScript* instance;
        EventMap events;
        bool rotatedirection;

        void Reset()
        {
            DoZoneInCombat(me, 100.0f);
        }

        void EnterCombat(Unit* who){}

        void EnterEvadeMode(){}

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
            case DATA_CREATE_MIST:
                if (Creature* durumu = me->GetCreature(*me, instance->GetGuidData(NPC_DURUMU)))
                {
                    rotatedirection = durumu->AI()->GetData(DATA_GET_DURUMU_ROTATE_DIRECTION);
                    events.RescheduleEvent(EVENT_CREATE_MIST, data);
                }
                break;
            case DATA_CLOSE_SAFE_ZONE_IN_MIST:
                events.RescheduleEvent(EVENT_CLOSE_SAFE_ZONE_IN_MIST, data);
                break;
            }
        }

        void DamageTaken(Unit* attacker, uint32 &damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
                damage = 0;
        }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CREATE_MIST:
                case EVENT_CLOSE_SAFE_ZONE_IN_MIST:
                    DoCast(me, SPELL_TKR_WHOLE_SLICE_X_1, true);
                    break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_cross_eyeAI(pCreature);
    }
};

//133768
class spell_arterial_cut : public SpellScriptLoader
{
public:
    spell_arterial_cut() : SpellScriptLoader("spell_arterial_cut") { }

    class spell_arterial_cut_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_arterial_cut_AuraScript)

        void HandlePeriodicTick(AuraEffect const * /*aurEff*/)
        {
            if (GetTarget())
                if (GetTarget()->GetHealth() == GetTarget()->GetMaxHealth())
                    GetTarget()->RemoveAurasDueToSpell(SPELL_ARTERIAL_CUT);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_arterial_cut_AuraScript::HandlePeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_arterial_cut_AuraScript();
    }
};

//138467
class spell_lingering_gaze : public SpellScriptLoader
{
public:
    spell_lingering_gaze() : SpellScriptLoader("spell_lingering_gaze") { }

    class spell_lingering_gaze_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_lingering_gaze_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            if (GetCaster())
            {
                std::list<Player*>pllist;
                pllist.clear();
                GetPlayerListInGrid(pllist, GetCaster(), 100.0f);
                if (!pllist.empty())
                {
                    pllist.remove_if(LingeringGazeFilter());
                    if (!pllist.empty())
                        for (std::list<Player*>::const_iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                            GetCaster()->CastSpell(*itr, SPELL_LINGERING_GAZE_TR_M);
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_lingering_gaze_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_lingering_gaze_SpellScript();
    }
};

//134123, 134122, 134124
class spell_durumu_color_beam : public SpellScriptLoader
{
public:
    spell_durumu_color_beam() : SpellScriptLoader("spell_durumu_color_beam") { }

    class spell_durumu_color_beam_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_durumu_color_beam_AuraScript);

        void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                if (GetCaster() && GetCaster()->ToCreature())
                    GetCaster()->ToCreature()->AI()->DoAction(ACTION_CREATE_CONE);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_durumu_color_beam_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_durumu_color_beam_AuraScript();
    }
};

//133734, 133672, 133740
class spell_durumu_color_cone : public SpellScriptLoader
{
public:
    spell_durumu_color_cone() : SpellScriptLoader("spell_durumu_color_cone") { }

    class spell_durumu_color_cone_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_durumu_color_cone_AuraScript);

        void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            //if focus cone player died, need drop cone zone
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
                if (GetSpellInfo()->Id == SPELL_INFRARED_LIGHT_CONE || GetSpellInfo()->Id == SPELL_BLUE_RAY_CONE)
                    if (GetTarget() && GetCaster() && GetCaster()->ToCreature())
                        GetCaster()->ToCreature()->AI()->SetGUID(GetTarget()->GetGUID(), 2);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_durumu_color_cone_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_durumu_color_cone_AuraScript();
    }
};

//133732, 133677, 133738
class spell_durumu_color_cone_dmg : public SpellScriptLoader
{
public:
    spell_durumu_color_cone_dmg() : SpellScriptLoader("spell_durumu_color_cone_dmg") { }

    class spell_durumu_color_cone_dmg_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_durumu_color_cone_dmg_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (GetCaster() && GetCaster()->ToCreature())
            {
                if (targets.empty())
                {
                    if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                        if (Creature* durumu = GetCaster()->GetCreature(*GetCaster(), instance->GetGuidData(NPC_DURUMU)))
                            durumu->AI()->SetData(GetSpellInfo()->Id, 1);
                }
                else
                {
                    if (GetSpellInfo()->Id != SPELL_BRIGHT_LIGHT_CONE_DMG)
                    {
                        //if focus cone on creature (it means player drop it), who entrance in cone pick up him
                        if (GetCaster()->ToCreature()->AI()->GetData(DATA_IS_CONE_TARGET_CREATURE))
                        {
                            GetCaster()->ToCreature()->AI()->SetData(DATA_DESPAWN_CREATURE_CONE_TARGET, 0);
                            std::list<WorldObject*>::const_iterator itr = targets.begin();
                            std::advance(itr, urand(0, targets.size() - 1));
                            GetCaster()->ToCreature()->AI()->SetGUID((*itr)->GetGUID(), 3);
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_durumu_color_cone_dmg_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_110);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_durumu_color_cone_dmg_SpellScript();
    }
};

//136120 (red), 136119(blue)
class spell_durumu_color_creature_aura : public SpellScriptLoader
{
public:
    spell_durumu_color_creature_aura() : SpellScriptLoader("spell_durumu_color_creature_aura") { }

    class spell_durumu_color_creature_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_durumu_color_creature_aura_AuraScript);

        void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget() && GetTarget()->ToCreature())
                GetTarget()->ToCreature()->AI()->DoAction(ACTION_IN_CONE);
        }

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTarget() && GetTarget()->ToCreature() && GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_DEATH)
                GetTarget()->ToCreature()->AI()->DoAction(ACTION_NOT_IN_CONE);    
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_durumu_color_creature_aura_AuraScript::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectRemove += AuraEffectRemoveFn(spell_durumu_color_creature_aura_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_durumu_color_creature_aura_AuraScript();
    }
};

//136154
class spell_caustic_spike : public SpellScriptLoader
{
public:
    spell_caustic_spike() : SpellScriptLoader("spell_caustic_spike") { }

    class spell_caustic_spike_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_caustic_spike_SpellScript);

        void FilterTargets(std::list<WorldObject*> &targets)
        {
            if (targets.size() > 1)
                targets.resize(1);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_caustic_spike_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_caustic_spike_SpellScript();
    }
};

//136179
class spell_icy_grasp_aura : public SpellScriptLoader
{
public:
    spell_icy_grasp_aura() : SpellScriptLoader("spell_icy_grasp_aura") { }

    class spell_icy_grasp_aura_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_icy_grasp_aura_AuraScript);

        void OnTick(AuraEffect const* aurEff)
        {
            if (GetCaster())
                GetCaster()->CastSpell(GetCaster(), SPELL_ICY_GRASP, true);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_icy_grasp_aura_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_icy_grasp_aura_AuraScript();
    }
};

class IcyGraspFilter
{
public:
    bool operator()(WorldObject* unit) const
    {
        if (Player* pl = unit->ToPlayer())
            if (pl->HasAura(SPELL_BLUE_RAY_P_T_AURA))
                return false;
        return true;
    }
};

//136177
class spell_icy_grasp_dmg : public SpellScriptLoader
{
public:
    spell_icy_grasp_dmg() : SpellScriptLoader("spell_icy_grasp_dmg") { }

    class spell_icy_grasp_dmg_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_icy_grasp_dmg_SpellScript);

        void FilterTargets(std::list<WorldObject*> &targets)
        {
            if (!targets.empty())
                targets.remove_if(IcyGraspFilter());
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_icy_grasp_dmg_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_icy_grasp_dmg_SpellScript();
    }
};

//139107
class spell_mind_daggers_dmg : public SpellScriptLoader
{
public:
    spell_mind_daggers_dmg() : SpellScriptLoader("spell_mind_daggers_dmg") { }

    class spell_mind_daggers_dmg_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_mind_daggers_dmg_SpellScript);

        void FilterTargets(std::list<WorldObject*> &targets)
        {
            if (GetCaster())
                if (targets.size() > 10)
                    targets.resize(10);
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_mind_daggers_dmg_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_mind_daggers_dmg_SpellScript();
    }
};

//134169
class spell_disintegration_laser_prepare : public SpellScriptLoader
{
public:
    spell_disintegration_laser_prepare() : SpellScriptLoader("spell_disintegration_laser_prepare") { }

    class spell_disintegration_laser_prepare_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_disintegration_laser_prepare_AuraScript);

        void HandleEffectRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes mode)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE && GetCaster() && GetTarget())
            {
                GetCaster()->CastSpell(GetTarget(), SPELL_DISINTEGRATION_LASER);
                if (GetTarget() && GetTarget()->ToCreature())
                    GetTarget()->ToCreature()->AI()->DoAction(ACTION_LAUNCH_ROTATE);
            }
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_disintegration_laser_prepare_AuraScript::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_disintegration_laser_prepare_AuraScript();
    }
};

//136318
class spell_disintegration_laser_dummy : public SpellScriptLoader
{
public:
    spell_disintegration_laser_dummy() : SpellScriptLoader("spell_disintegration_laser_dummy") { }

    class spell_disintegration_laser_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_disintegration_laser_dummy_SpellScript);

        void DealDamage()
        {
            if (GetHitUnit())
            {
                if (InstanceScript* instance = GetHitUnit()->GetInstanceScript())
                {
                    if (Creature* durumu = GetHitUnit()->GetCreature(*GetHitUnit(), instance->GetGuidData(NPC_DURUMU)))
                        if (Creature* drumueyetarget = GetHitUnit()->GetCreature(*GetHitUnit(), instance->GetGuidData(NPC_EYEBEAM_TARGET_DURUMU)))
                            if (GetHitUnit()->IsInBetween(durumu, drumueyetarget, 6.0f))
                                GetHitUnit()->Kill(GetHitUnit(), true);
                }
            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_disintegration_laser_dummy_SpellScript::DealDamage);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_disintegration_laser_dummy_SpellScript();
    }
};

//136932
class spell_force_of_will_dummy : public SpellScriptLoader
{
public:
    spell_force_of_will_dummy() : SpellScriptLoader("spell_force_of_will_dummy") { }

    class spell_force_of_will_dummy_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_force_of_will_dummy_AuraScript);

        void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetCaster() && GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                GetCaster()->CastSpell(GetCaster(), SPELL_FORCE_OF_WILL_KNOCK_B, true);
        }

        void Register()
        {
            OnEffectRemove += AuraEffectRemoveFn(spell_force_of_will_dummy_AuraScript::HandleRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_force_of_will_dummy_AuraScript();
    }
};

class ForceofWillFilter
{
public:
    ForceofWillFilter(WorldObject* caster) : _caster(caster){}

    bool operator()(WorldObject* unit)
    {
        if (_caster->isInFront(unit, M_PI / 6))
            return false;
        return true;
    }
private:
    WorldObject* _caster;
};

//136413
class spell_force_of_will : public SpellScriptLoader
{
public:
    spell_force_of_will() : SpellScriptLoader("spell_force_of_will") { }

    class spell_force_of_will_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_force_of_will_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (GetCaster())
                targets.remove_if(ForceofWillFilter(GetCaster()));
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_force_of_will_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENEMY_104);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_force_of_will_SpellScript();
    }
};

//133798
class spell_durumu_life_drain : public SpellScriptLoader
{
public:
    spell_durumu_life_drain() : SpellScriptLoader("spell_durumu_life_drain") { }

    class spell_durumu_life_drain_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_durumu_life_drain_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (GetCaster())
            {
                targets.clear();
                ObjectGuid maintargetGuid;
                std::list<Player*> pllist;
                pllist.clear();
                GetPlayerListInGrid(pllist, GetCaster(), 100.0f);

                if (pllist.empty())
                    return;

                for (std::list<Player*>::iterator itr = pllist.begin(); itr != pllist.end();)
                {
                    if ((*itr)->HasAura(SPELL_LIFE_DRAIN_STUN))
                    {
                        maintargetGuid = (*itr)->GetGUID();
                        pllist.erase(itr);
                        break;
                    }
                    else
                        ++itr;
                }

                if (!maintargetGuid)
                    return;

                if (!pllist.empty()) //check if anybody stay between caster and target
                {
                    if (Player* mtarget = GetCaster()->GetPlayer(*GetCaster(), maintargetGuid))
                    {
                        for (std::list<Player*>::iterator itr = pllist.begin(); itr != pllist.end();)
                        {
                            if (!(*itr)->IsInBetween(GetCaster(), mtarget, 3.0f))
                                pllist.erase(itr++);
                            else
                                ++itr;
                        }
                    }
                }

                if (pllist.size() > 1)
                {
                    ObjectGuid newtargetGuid;
                    float range = 100; //max spell range

                    for (std::list<Player*>::iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                        if (GetCaster()->GetExactDist2d((*itr)) < range)
                            range = GetCaster()->GetExactDist2d((*itr));

                    for (std::list<Player*>::iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                        if (GetCaster()->GetExactDist2d((*itr)) <= range)
                            newtargetGuid = (*itr)->GetGUID();

                    if (!newtargetGuid)
                        return;

                    if (Player* target = GetCaster()->GetPlayer(*GetCaster(), newtargetGuid))
                        targets.push_back(target);
                }
                else if (pllist.size() == 1)
                {
                    for (std::list<Player*>::iterator itr = pllist.begin(); itr != pllist.end(); itr++)
                        targets.push_back(*itr);
                }
                else if (pllist.empty())
                {
                    if (Player* target = GetCaster()->GetPlayer(*GetCaster(), maintargetGuid))
                        targets.push_back(target);
                }
            }
        }

        void DealDamage()
        {
            if (GetCaster() && GetHitUnit())
            {
                uint32 dmg = GetHitDamage(); //default;
                if (GetHitUnit()->HasAura(SPELL_LIFE_DRAIN_DMG))
                {
                    uint8 stack = GetHitUnit()->GetAura(SPELL_LIFE_DRAIN_DMG)->GetStackAmount();
                    uint32 mod = GetHitDamage() * 0.6; //increase on 60% from stack
                    uint32 moddmg = mod*stack;
                    dmg = dmg + moddmg;
                    SetHitDamage(dmg);
                }
                else
                    SetHitDamage(dmg);

                uint32 healcount = dmg * 0.25f; //25% from dmg
                if (InstanceScript* instance = GetCaster()->GetInstanceScript())
                    if (Creature* durumu = GetCaster()->GetCreature(*GetCaster(), instance->GetGuidData(NPC_DURUMU)))
                        if (durumu->isAlive())
                            GetCaster()->CastCustomSpell(SPELL_LIFE_DRAIN_HEAL, SPELLVALUE_BASE_POINT0, healcount, durumu, true);
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_durumu_life_drain_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_durumu_life_drain_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            OnHit += SpellHitFn(spell_durumu_life_drain_SpellScript::DealDamage);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_durumu_life_drain_SpellScript();
    }
};

//8897
class at_durumu_entrance : public AreaTriggerScript
{
public:
    at_durumu_entrance() : AreaTriggerScript("at_durumu_entrance") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* areaTrigger, bool enter)
    {
        if (enter)
            player->RemoveAurasDueToSpell(SPELL_JIKUN_FLY);
        return true;
    }
};

void AddSC_boss_durumu()
{
    new boss_durumu();
    new npc_appraising_eye();
    new npc_mind_eye();
    new npc_hungry_eye();
    new npc_colorblind_eye();
    new npc_colorblind_eye_beam_target();
    new npc_durumu_fog();
    new npc_durumu_eyebeam_target();
    new npc_cross_eye();
    new spell_arterial_cut();
    new spell_lingering_gaze();
    new spell_durumu_color_beam();
    new spell_durumu_color_cone();
    new spell_durumu_color_cone_dmg();
    new spell_durumu_color_creature_aura();
    new spell_caustic_spike();
    new spell_icy_grasp_aura();
    new spell_icy_grasp_dmg();
    new spell_mind_daggers_dmg();
    new spell_disintegration_laser_prepare();
    new spell_disintegration_laser_dummy();
    new spell_force_of_will_dummy();
    new spell_force_of_will();
    new spell_durumu_life_drain();
    new at_durumu_entrance();
}
