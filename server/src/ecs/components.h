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

#pragma once

#include <string>
#include <variant>
#include <array>
#include <vector>
#include <optional>
#include <type_traits>
#include <ibh_containers.h>
#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>
#include "common_components.h"

using namespace std;

namespace ibh {

    // enums

    // components

    struct random_stat_component {
        string name;
        int64_t min;
        int64_t max;

        random_stat_component(string name, int64_t min, int64_t max) : name(move(name)), min(min), max(max) {}
    };

    struct skill_component {
        string name;
        int64_t level;
    };

    struct item_effect_component {
        string name;
        string tooltip;
        string message;
        uint32_t potency;
        uint32_t duration;
        uint32_t uses;
        uint32_t range;
        uint32_t chance;
        bool autocast;
        bool can_apply;
        vector<stat_component> stats;
    };

    struct item_required_skill_component {
        string name;
        uint32_t level;
    };

    struct item_component {
        string name;
        string desc;
        string type;
        uint64_t value;
//        uint64_t sprite;
        uint64_t quality;
        uint64_t enchant_level;
        uint64_t required_level;
        uint64_t tier;
        bool binds;
        bool tells_bind;
//        optional<item_effect_component> effect;
//        optional<random_stat_component> random_trait_levels;
//        vector<string> random_trait_names;
//        vector<string> required_professions;
//        vector<item_required_skill_component> required_skills;
        vector<stat_component> stats;
//        vector<random_stat_component> random_stats;

        item_component() : name(), desc(), type(), value(), quality(), enchant_level(), required_level(), tier(), binds(), tells_bind(), stats() {}
        item_component(string name, string desc, string type, uint64_t value, uint64_t quality, uint64_t enchant_level, uint64_t required_level, uint64_t tier, bool binds, bool tells_bind, vector<stat_component> stats)
        : name(move(name)), desc(move(desc)), type(move(type)), value(value), quality(quality), enchant_level(enchant_level), required_level(required_level), tier(tier), binds(binds), tells_bind(tells_bind), stats(move(stats)) {}
    };

    struct monster_definition_component {
        string name;

        ibh_flat_map<uint32_t, int64_t> stats;
        //vector<random_stat_component> random_stats;
        //vector<item_component> items;
        //vector<skill_component> skills;

        monster_definition_component(string name, ibh_flat_map<uint32_t, int64_t> stats) :
        name(move(name)), stats(move(stats)) {}
    };

    struct monster_special_definition_component {
        string name;
        ibh_flat_map<uint32_t, int64_t> stats;
        bool teleport_when_beat;

        monster_special_definition_component(string name, ibh_flat_map<uint32_t, int64_t> stats, bool teleport) : name(move(name)), stats(move(stats)), teleport_when_beat(teleport) {}
    };

    struct monster_component {
        string name;
        string special_name;
        uint32_t level;
        ibh_flat_map<uint32_t, int64_t> stats;
        bool teleport_when_beat;
    };

    struct company_building_definition_component {
        string name;
        vector<stat_component> bonuses;
        uint64_t cost;
    };

    struct company_component {
        uint64_t id;
        uint16_t member_level;
        string name;
        ibh_flat_map<uint32_t, int64_t> stats;
    };

    struct battle_component {
        bool done;
        string monster_name;
        uint32_t monster_level;
        ibh_flat_map<uint32_t, int64_t> monster_stats;
        ibh_flat_map<uint32_t, int64_t> total_player_stats;

        battle_component() : done(true), monster_name(), monster_level(), monster_stats(), total_player_stats() {}
        battle_component(string monster_name, uint32_t monster_level, ibh_flat_map<uint32_t, int64_t> monster_stats) : done(false), monster_name(move(monster_name)), monster_level(monster_level), monster_stats(move(monster_stats)) {}
    };

    struct pc_component {
        uint64_t id;
        uint64_t connection_id;
        string name;
        string race;
        string dir;
        string _class;
        string spawn_message;

        uint64_t level;
        uint64_t skill_points;

        ibh_flat_map<uint32_t, int64_t> stats;
        ibh_flat_map<uint32_t, item_component> equipped_items;
        vector<item_component> inventory;
        ibh_flat_map<string, skill_component> skills;

        pc_component() : id(), connection_id(), name(), race(), dir(), _class(), spawn_message(),
                          level(), skill_points(), stats(), equipped_items(), inventory(), skills() {}
        pc_component(uint64_t id, uint64_t connection_id, string name, string race, string dir, string _class, string spawn_message, uint64_t level, uint64_t skill_points, ibh_flat_map<uint32_t, int64_t> stats, ibh_flat_map<uint32_t, item_component> equipped_items, vector<item_component> inventory, ibh_flat_map<string, skill_component> skills)
        : id(id), connection_id(connection_id), name(move(name)), race(move(race)), dir(move(dir)), _class(move(_class)), spawn_message(move(spawn_message)),
                          level(level), skill_points(skill_points), stats(move(stats)), equipped_items(move(equipped_items)), inventory(move(inventory)), skills(move(skills)) {}
    };

    struct user_component {
        string name;
        string email;
        uint16_t subscription_tier;
        bool is_tester;
        bool is_game_master;
        uint16_t trial_ends;
        bool has_done_trial;
        string discord_tag;
        bool discord_online;
        vector<pc_component> characters;
    };

    struct wood_gathering_component {};
    struct ore_gathering_component {};
    struct water_gathering_component {};
    struct plants_gathering_component {};
    struct clay_gathering_component {};
    struct paper_gathering_component {};
    struct ink_gathering_component {};
    struct metal_gathering_component {};
    struct bricks_gathering_component {};
    struct gems_gathering_component {};
    struct timber_gathering_component {};
    struct item_gathering_component {};
    struct working_component {};

    // helper functions
    [[nodiscard]]
    auto get_stat(decltype(pc_component::stats) &stats, decltype(pc_component::stats)::key_type stat_id) -> decltype(pc_component::stats)::mapped_type&;

    auto get_stat_or_initialize_default(decltype(pc_component::stats) &stats, decltype(pc_component::stats)::key_type stat_id, decltype(pc_component::stats)::mapped_type default_val) -> decltype(pc_component::stats)::mapped_type&;

    // constants

    // company member levels

}
