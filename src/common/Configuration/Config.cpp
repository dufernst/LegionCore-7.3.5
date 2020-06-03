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

#include <algorithm>
#include <mutex>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "Config.h"

namespace
{
    boost::property_tree::ptree _config;
    std::string _filename;
    std::mutex _configLock;
}

namespace bpt = boost::property_tree;

bool ConfigMgr::LoadInitial(std::string const& file, std::string& error)
{
    std::lock_guard<std::mutex> lock(_configLock);

    _filename = file;

    try
    {
        bpt::ptree fullTree;
        read_ini(file, fullTree);

        if (fullTree.empty())
        {
            error = "empty file (" + file + ")";
            return false;
        }

        // Since we're using only one section per config file, we skip the section and have direct property access
        _config = fullTree.begin()->second;
    }
    catch (bpt::ini_parser::ini_parser_error const& e)
    {
        if (e.line() == 0)
            error = e.message() + " (" + e.filename() + ")";
        else
            error = e.message() + " (" + e.filename() + ":" + std::to_string(e.line()) + ")";
        return false;
    }

    return true;
}

ConfigMgr* ConfigMgr::instance()
{
    static ConfigMgr instance;
    return &instance;
}

bool ConfigMgr::Reload(std::string& error)
{
    return LoadInitial(_filename, error);
}

std::string ConfigMgr::GetStringDefault(std::string const& name, const std::string& def)
{
    auto value = _config.get<std::string>(bpt::ptree::path_type(name, '/'), def);
    value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
    return value;
}

bool ConfigMgr::GetBoolDefault(std::string const& name, bool def)
{
    try
    {
        auto val = _config.get<std::string>(bpt::ptree::path_type(name, '/'));
        val.erase(std::remove(val.begin(), val.end(), '"'), val.end());
        return (val == "true" || val == "TRUE" || val == "yes" || val == "YES" || val == "1");
    }
    catch (std::exception const& /*ex*/)
    {
        return def;
    }
}

int ConfigMgr::GetIntDefault(std::string const& name, int def)
{
    return _config.get<int>(bpt::ptree::path_type(name, '/'), def);
}

float ConfigMgr::GetFloatDefault(std::string const& name, float def)
{
    return _config.get<float>(bpt::ptree::path_type(name, '/'), def);
}

std::string const& ConfigMgr::GetFilename()
{
    std::lock_guard<std::mutex> lock(_configLock);
    return _filename;
}

std::vector<std::string> ConfigMgr::GetKeysByString(std::string const& name)
{
    std::lock_guard<std::mutex> lock(_configLock);

    std::vector<std::string> keys;
    for (const auto& child : _config)
        if (child.first.compare(0, name.length(), name) == 0)
            keys.push_back(child.first);

    return keys;
}

