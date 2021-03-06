/*
    IdleBossHunter
    Copyright (C) 2020 Michael de Lang

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "load_monster_specials.h"
#include <working_directory_manipulation.h>
#include "spdlog/spdlog.h"
#include <ecs/components.h>
#include <rapidjson/document.h>

using namespace std;
using namespace ibh;
using namespace rapidjson;

optional<monster_special_definition_component> ibh::load_monster_specials(string const &file) {
    auto env_contents = read_whole_file(file);

    if(!env_contents) {
        spdlog::trace("[{}] couldn't load file {}", __FUNCTION__, file);
        return {};
    }

    Document d;
    d.Parse(env_contents->c_str(), env_contents->size());

    if(!d.IsObject() || !d.HasMember("name") || !d.HasMember("multipliers") || !d.HasMember("teleport_when_beat")) {
        spdlog::trace("[{}] couldn't load, missing members file {}", __FUNCTION__, file);
        return {};
    }

    ibh_flat_map<uint32_t, int64_t> stats;
    stats.reserve(d["multipliers"].MemberCount());
    string name = d["name"].GetString();

    for (auto const &stat : stat_names) {
        if(d["multipliers"].HasMember(stat.c_str())) {
            auto mapper_it = stat_name_to_id_mapper.find(stat);
            if(mapper_it == end(stat_name_to_id_mapper)) {
                spdlog::error("[{}] monster {} could not map stat {}", __FUNCTION__, name, stat);
                continue;
            }
            stats.emplace(mapper_it->second, d["multipliers"][stat.c_str()].GetInt64());
        }
    }

    return monster_special_definition_component(name, move(stats), d["teleport_when_beat"].GetBool());
}
