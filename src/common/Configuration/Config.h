/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#ifndef CONFIG_H
#define CONFIG_H

#include "Define.h"
#include <vector>

class ConfigMgr
{
    ConfigMgr() = default;
    ~ConfigMgr() = default;

public:

    ConfigMgr(ConfigMgr const&) = delete;
    ConfigMgr& operator=(ConfigMgr const&) = delete;

    bool LoadInitial(std::string const& file, std::string& error);

    static ConfigMgr* instance();

    bool Reload(std::string& error);

    std::string GetStringDefault(std::string const& name, const std::string& def);
    bool GetBoolDefault(std::string const& name, bool def);
    int GetIntDefault(std::string const& name, int def);
    float GetFloatDefault(std::string const& name, float def);

    std::string const& GetFilename();
    std::vector<std::string> GetKeysByString(std::string const& name);
};

#define sConfigMgr ConfigMgr::instance()

#endif
