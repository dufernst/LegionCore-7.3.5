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

#ifndef DEF_SCARLET_H
#define DEF_SCARLET_H 

enum Data
{
    DATA_BRAUN      = 0,
    DATA_HARLAN     = 1,
    DATA_KOEGLER    = 2,
    MAX_ENCOUNTER,
};

enum eCreatures
{
    NPC_HOUNDMASTER_BRAUN    = 59303,
    NPC_ARMSMASTER_HARLAN    = 58632,
    NPC_FLAMEWEAVER_KOEGLER  = 59150,
    //Braun summons
    NPC_OBEDIENT_HOUND       = 59309,
    //Koegler summons
    NPC_DRAGON_BREATH_TARGET = 59198,
};

enum eGameObjects
{
    GO_BRAUN_DOOR       = 210097,
    GO_HARLAN_DOOR      = 210480,
};
#endif
