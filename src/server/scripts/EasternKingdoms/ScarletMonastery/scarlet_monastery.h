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

#ifndef DEF_SCARLET_M
#define DEF_SCARLET_M

enum eEnums
{
    DATA_THALNOS                = 1,
    DATA_KORLOFF                = 2,
    DATA_WHITEMANE              = 3,
    DATA_DURAND                 = 4,
    MAX_ENCOUNTER,

    DATA_HORSEMAN_EVENT         = 6,
    GAMEOBJECT_PUMPKIN_SHRINE   = 7,
};

enum eCreatures
{
    NPC_HORSEMAN            = 23682,
    NPC_HEAD                = 23775,
    NPC_PUMPKIN             = 23694,
    NPC_THALNOS             = 59789,
    NPC_BROTHER_KORLOFF     = 59223,
    NPC_WHITEMANE           = 3977,
    
    //Summons Thalnos
    NPC_EVICTED_SOUL        = 59974,
    NPC_EMPOWERING_SPIRIT   = 59893,
    NPC_FALLEN_CRUSADER     = 59884,
    NPC_EMPOWERED_ZOMBIE    = 59930,

    //Summons Korloff
    NPC_TRAINING_DUMMY      = 64446,
    NPC_SCORCHED_EARTH      = 59507,

    //Summons Whitemane
    NPC_DURAND              = 60040,
    NPC_SCARLET_JUDICATOR   = 58605,
};

enum eGameObects
{
    GO_THALNOS_DOOR      = 211844,
    GO_KORLOFF_DOOR      = 210564,
    GO_WHITEMANE_DOOR    = 210563,
    GO_PUMPKIN_SHRINE    = 186267,
};
#endif
