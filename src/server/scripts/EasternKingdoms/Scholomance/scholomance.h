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

#ifndef DEF_SCHOLOMANCE_H
#define DEF_SCHOLOMANCE_H

#include "SpellScript.h"
#include "Map.h"
#include "Creature.h"
#include "CreatureAIImpl.h"

enum DataTyped
{ 
    DATA_INSTRUCTOR           = 0,
    DATA_BAROV                = 1,
    DATA_RATTLEGORE           = 2,
    DATA_LILIAN               = 3,
    DATA_DARKMASTER           = 4
};

enum CreatureIds
{
    NPC_INSTRUCTOR_CHILLHEART = 58633,
    NPC_JANDICE_BAROV         = 59184,
    NPC_RATTLEGORE            = 59153,
    NPC_LILIAN_VOSS           = 59200,
    NPC_DARKMASTER_GANDLING   = 59080
};

#endif

