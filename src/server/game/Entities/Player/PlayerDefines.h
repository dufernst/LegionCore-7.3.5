/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
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

#ifndef PlayerDefines_h__
#define PlayerDefines_h__

#include "Define.h"

enum CharacterFlags : uint32
{
    CHARACTER_FLAG_NONE                 = 0x00000000,
    CHARACTER_FLAG_UNK1                 = 0x00000001,
    CHARACTER_FLAG_UNK2                 = 0x00000002,
    CHARACTER_FLAG_LOCKED_FOR_TRANSFER  = 0x00000004,   // You cannot log in until the character update process you recently initiated is complete.
    CHARACTER_FLAG_UNK4                 = 0x00000008,
    CHARACTER_FLAG_UNK5                 = 0x00000010,
    CHARACTER_FLAG_UNK6                 = 0x00000020,
    CHARACTER_FLAG_UNK7                 = 0x00000040,
    CHARACTER_FLAG_UNK8                 = 0x00000080,
    CHARACTER_FLAG_UNK9                 = 0x00000100,
    CHARACTER_FLAG_UNK10                = 0x00000200,
    CHARACTER_FLAG_HIDE_HELM            = 0x00000400,
    CHARACTER_FLAG_HIDE_CLOAK           = 0x00000800,
    CHARACTER_FLAG_UNK13                = 0x00001000,
    CHARACTER_FLAG_GHOST                = 0x00002000,
    CHARACTER_FLAG_RENAME               = 0x00004000,
    CHARACTER_FLAG_UNK16                = 0x00008000,
    CHARACTER_FLAG_UNK17                = 0x00010000,
    CHARACTER_FLAG_UNK18                = 0x00020000,
    CHARACTER_FLAG_UNK19                = 0x00040000,
    CHARACTER_FLAG_UNK20                = 0x00080000,
    CHARACTER_FLAG_UNK21                = 0x00100000,
    CHARACTER_FLAG_UNK22                = 0x00200000,
    CHARACTER_FLAG_UNK23                = 0x00400000,
    CHARACTER_FLAG_UNK24                = 0x00800000,
    CHARACTER_FLAG_LOCKED_BY_BILLING    = 0x01000000,   // <html><body><p align=\"CENTER\">Character Locked.\nSee <a href=\"https://www.battle.net/support/article/6592\">https://www.battle.net/support/article/6592</a> for more information.</p></body></html>
    CHARACTER_FLAG_DECLINED             = 0x02000000,
    CHARACTER_FLAG_UNK27                = 0x04000000,
    CHARACTER_FLAG_UNK28                = 0x08000000,
    CHARACTER_FLAG_UNK29                = 0x10000000,
    CHARACTER_FLAG_UNK30                = 0x20000000,
    CHARACTER_FLAG_UNK31                = 0x40000000,
    CHARACTER_FLAG_UNK32                = 0x80000000
};

enum CharacterFlags2 : uint32
{
    CHARACTER_FLAG_2_NONE                                   = 0x00000000,
    CHARACTER_FLAG_2_CUSTOMIZE                              = 0x00000001,       // name, gender, etc...
    CHARACTER_FLAG_2_2                                      = 0x00000002,
    CHARACTER_FLAG_2_24                                     = 0x00008000,
    CHARACTER_FLAG_2_FACTION                                = 0x00010000,       // name, gender, faction, etc...
    CHARACTER_FLAG_2_RACE                                   = 0x00100000,       // name, gender, race, etc...
    CHARACTER_FLAG_2_RACE_CHANGE_DISABLED                   = 0x10000000,       // Paid Race Change is currently disabled for this character.|nThis is due to not finishing the quests in this character's starting area.
};

enum CharacterFlags3 : uint32
{
    CHARACTER_FLAG_3_LOCKED_BY_REVOKED_VAS_TRANSACTION      = 0x00100000,
    CHARACTER_FLAG_3_LOCKED_BY_REVOKED_CHARACTER_UPGRADE    = 0x80000000,
};

enum PlayerFlags
{
    PLAYER_FLAGS_GROUP_LEADER           = 0x00000001,
    PLAYER_FLAGS_AFK                    = 0x00000002,
    PLAYER_FLAGS_DND                    = 0x00000004,
    PLAYER_FLAGS_GM                     = 0x00000008,
    PLAYER_FLAGS_GHOST                  = 0x00000010,
    PLAYER_FLAGS_RESTING                = 0x00000020,
    PLAYER_FLAGS_UNK6                   = 0x00000040,
    PLAYER_FLAGS_UNK7                   = 0x00000080,       // pre-3.0.3 PLAYER_FLAGS_FFA_PVP flag for FFA PVP state
    PLAYER_FLAGS_CONTESTED_PVP          = 0x00000100,       // Player has been involved in a PvP combat and will be attacked by contested guards
    PLAYER_FLAGS_IN_PVP                 = 0x00000200,
    PLAYER_FLAGS_HIDE_HELM              = 0x00000400,
    PLAYER_FLAGS_HIDE_CLOAK             = 0x00000800,
    PLAYER_FLAGS_PLAYED_LONG_TIME       = 0x00001000,       // played long time
    PLAYER_FLAGS_PLAYED_TOO_LONG        = 0x00002000,       // played too long time
    PLAYER_FLAGS_IS_OUT_OF_BOUNDS       = 0x00004000,
    PLAYER_FLAGS_DEVELOPER              = 0x00008000,       // <Dev> prefix for something?
    PLAYER_FLAGS_UNK16                  = 0x00010000,       // pre-3.0.3 PLAYER_FLAGS_SANCTUARY flag for player entered sanctuary
    PLAYER_FLAGS_TAXI_BENCHMARK         = 0x00020000,       // taxi benchmark mode (on/off) (2.0.1)
    PLAYER_FLAGS_PVP_TIMER              = 0x00040000,       // 3.0.2, pvp timer active (after you disable pvp manually)
    PLAYER_FLAGS_COMMENTATOR            = 0x00080000,
    PLAYER_FLAGS_UNK20                  = 0x00100000,
    PLAYER_FLAGS_UNK21                  = 0x00200000,
    PLAYER_FLAGS_COMMENTATOR_UBER       = 0x00400000,
    PLAYER_ALLOW_ONLY_ABILITY           = 0x00800000,       // used by bladestorm and killing spree, allowed only spells with SPELL_ATTR0_REQ_AMMO, SPELL_EFFECT_ATTACK, checked only for active player
    PLAYER_FLAGS_PET_BATTLES_UNLOCKED   = 0x01000000,       // pet battles
    PLAYER_FLAGS_NO_XP_GAIN             = 0x02000000,
    PLAYER_FLAGS_UNK26                  = 0x04000000,
    PLAYER_FLAGS_AUTO_DECLINE_GUILD     = 0x08000000,       // Automatically declines guild invites
    PLAYER_FLAGS_GUILD_LEVEL_ENABLED    = 0x10000000,       // Lua_GetGuildLevelEnabled() - enables guild leveling related UI
    PLAYER_FLAGS_VOID_UNLOCKED          = 0x20000000,       // void storage
    PLAYER_FLAGS_MENTOR                 = 0x40000000,
    PLAYER_FLAGS_COMMENTATOR2           = 0x80000000
};

enum PlayerFlagsEx
{
    PLAYER_FLAGS_EX_REAGENT_BANK_UNLOCKED   = 0x0001,
    PLAYER_FLAGS_EX_MERCENARY_MODE          = 0x0002
};

enum PlayerLocalFlags
{
    PLAYER_LOCAL_FLAG_CONTROLLING_PET               = 0x00000001,
    PLAYER_LOCAL_FLAG_TRACK_STEALTHED               = 0x00000002,
    PLAYER_LOCAL_FLAG_RELEASE_TIMER                 = 0x00000008,       // Display time till auto release spirit
    PLAYER_LOCAL_FLAG_NO_RELEASE_WINDOW             = 0x00000010,       // Display no "release spirit" window at all
    PLAYER_LOCAL_FLAG_NO_PET_BAR                    = 0x00000020,       // CGPetInfo::IsPetBarUsed
    PLAYER_LOCAL_FLAG_OVERRIDE_CAMERA_MIN_HEIGHT    = 0x00000040,
    PLAYER_LOCAL_FLAG_NEWLY_BOOSTED_CHARACTER       = 0x00000080,
    PLAYER_LOCAL_FLAG_USING_PARTY_GARRISON          = 0x00000100,
    PLAYER_LOCAL_FLAG_CAN_USE_OBJECTS_MOUNTED       = 0x00000200,
    PLAYER_LOCAL_FLAG_CAN_VISIT_PARTY_GARRISON      = 0x00000400,
    PLAYER_LOCAL_FLAG_ACCOUNT_SECURED               = 0x00001000,   // Script_IsAccountSecured
};

#endif // PlayerDefines_h__
