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

#include "show_characters_scene.h"
#include "messages/user_access/play_character_request.h"
#include "messages/user_access/play_character_response.h"
#include "messages/user_access/create_character_request.h"
#include "messages/user_access/create_character_response.h"
#include "messages/user_access/delete_character_request.h"
#include "messages/user_access/delete_character_response.h"
#include "messages/user_access/character_select_request.h"
#include "messages/user_access/character_select_response.h"
#include "messages/generic_error_response.h"
#include "messages/update_response.h"
#include "ibh_containers.h"
#include <rendering/imgui/imgui.h>
#include "spdlog/spdlog.h"
#include <SDL.h>
#include <algorithm>
#include "scenes/gui_scenes/resources/battle_log_scene.h"

using namespace std;
using namespace ibh;

show_characters_scene::show_characters_scene(iscene_manager *manager, vector<character_object> characters) : scene(generate_type<show_characters_scene>()), _characters(move(characters)), _races(), _classes(), _show_create(false), _waiting_for_select(true),
_waiting_for_reply(false), _error(), _selected_race(), _selected_class(), _selected_slot(0), _selected_play_slot(-1) {
    send_message<character_select_request>(manager);
}

void show_characters_scene::update(iscene_manager *manager, TimeDelta dt) {
    if(_waiting_for_select) {
        ImGui::Begin("Waiting for server", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::End();
        return;
    }

    if(ImGui::Begin("Characters List", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if(!_error.empty()) {
            ImGui::Text("%s", _error.c_str());
        }

        if(ImGui::ListBoxHeader("Characters", _characters.size(), 4)) {
            for (auto &character : _characters) {
                if (ImGui::Selectable(fmt::format("{} {}", character.name, character.level).c_str(), static_cast<uint32_t>(_selected_play_slot) == character.slot)) {
                    _selected_play_slot = character.slot;
                }
            }
            ImGui::ListBoxFooter();
        }

        if (ImGui::Button("Create Character")) {
            _show_create = true;
        }

        disable_buttons_when(_waiting_for_reply || _selected_play_slot < 0);

        ImGui::SameLine();

        if (ImGui::Button("Delete Character")) {
            _show_delete = true;
        }

        if (ImGui::Button("Play")) {
            send_message<play_character_request>(manager, static_cast<uint32_t>(_selected_play_slot));
            _waiting_for_reply = true;
        }

        reenable_buttons();
    }
    ImGui::End();

    if(_show_delete) {
        if(ImGui::Begin("Delete Character", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            if(!_error.empty()) {
                ImGui::Text("%s", _error.c_str());
            }

            for(auto& character : _characters) {
                if(character.slot == _selected_slot) {
                    ImGui::Text("Are you sure you want to delete character %s in slot %i?", _error.c_str(), _selected_slot);
                    break;
                }
            }

            disable_buttons_when(_waiting_for_reply);

            if (ImGui::Button("Yes")) {
                send_message<delete_character_request>(manager, static_cast<uint32_t>(_selected_play_slot));
                _waiting_for_reply = true;
            }

            if (ImGui::Button("No")) {
                _show_delete = false;
            }

            reenable_buttons();
        }
        ImGui::End();
    }

    if(!_show_create) {
        return;
    }

    if(ImGui::Begin("Create Character", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if(!_error.empty()) {
            ImGui::Text("%s", _error.c_str());
        }

        static char bufcharname[64];
        ImGui::InputTextWithHint("Character name", "<character name>", bufcharname, 64, ImGuiInputTextFlags_AutoSelectAll);

        if (ImGui::BeginCombo("Race", _selected_race.c_str()))
        {
            for(auto const &race : _races) {
                if (ImGui::Selectable(race.name.c_str(), _selected_race == race.name)) {
                    _selected_race = race.name;
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Class", _selected_class.c_str()))
        {
            for(auto const &c : _classes) {
                if (ImGui::Selectable(c.name.c_str(), _selected_class == c.name)) {
                    _selected_class = c.name;
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Slot", fmt::format("{}", _selected_slot).c_str()))
        {
            for(uint32_t i = 0; i < 4; i++) {
                if (ImGui::Selectable(fmt::format("{}", i).c_str(), _selected_slot == i)) {
                    _selected_slot = i;
                }
            }
            ImGui::EndCombo();
        }

        disable_buttons_when(_selected_class.empty() || _selected_race.empty() || strlen(bufcharname) < 2 || _waiting_for_reply);

        if (ImGui::Button("Create")) {
            _waiting_for_reply = true;
            send_message<create_character_request>(manager, _selected_slot, bufcharname, _selected_race, _selected_class);
        }

        reenable_buttons();

        if(!_selected_class.empty() && !_selected_race.empty()) {
            ibh_flat_map<uint64_t, int64_t> combined_stats;

            vector<character_race>::iterator r = find_if(begin(_races), end(_races), [&_selected_race = _selected_race](const character_race &race){return race.name == _selected_race;});
            vector<character_class>::iterator c = find_if(begin(_classes), end(_classes), [&_selected_class = _selected_class](const character_class &_class){return _class.name == _selected_class;});

            if(r == end(_races) || c == end(_classes)) {
                spdlog::error("[{}] something went wrong with calculating stats. Please report this as a bug.", __FUNCTION__);
                ImGui::End();
                return;
            }

            for(auto const &stat : r->level_stat_mods) {
                auto stat_name_it = stat_id_to_name_mapper.find(stat.stat_id);
                if(stat_name_it == end(stat_id_to_name_mapper)) {
                    throw runtime_error("");
                }
                ImGui::Text("%s %s: %lli", r->name.c_str(), stat_name_it->second.c_str(), stat.value) ;
                combined_stats[stat.stat_id] += stat.value;
            }

            for(auto const &stat : c->stat_mods) {
                auto stat_name_it = stat_id_to_name_mapper.find(stat.stat_id);
                if(stat_name_it == end(stat_id_to_name_mapper)) {
                    throw runtime_error("");
                }
                ImGui::Text("%s %s: %lli", c->name.c_str(), stat_name_it->second.c_str(), stat.value);
                combined_stats[stat.stat_id] += stat.value;
            }

            for(auto const &stat : c->stat_mods) {
                auto stat_name_it = stat_id_to_name_mapper.find(stat.stat_id);
                if(stat_name_it == end(stat_id_to_name_mapper)) {
                    throw runtime_error("");
                }
                ImGui::Text("%s: %lli", stat_name_it->second.c_str(), combined_stats[stat.stat_id]);
            }
        }
    }
    ImGui::End();
}

void show_characters_scene::handle_message(iscene_manager *manager, uint64_t type, message const *msg) {
    switch (type) {
        case update_response::type: {
            auto resp_msg = dynamic_cast<update_response const *>(msg);

            if (!resp_msg) {
                return;
            }

            _closed = true;
            break;
        }
        case character_select_response::type: {
            auto resp_msg = dynamic_cast<character_select_response const *>(msg);

            if (!resp_msg) {
                return;
            }

            _races = resp_msg->races;
            _classes = resp_msg->classes;
            _waiting_for_select = false;
            break;
        }
        case create_character_response::type: {
            auto resp_msg = dynamic_cast<create_character_response const *>(msg);

            if (!resp_msg) {
                return;
            }

            _characters.emplace_back(resp_msg->character);
            _waiting_for_reply = false;
            _show_create = false;
            break;
        }
        case delete_character_response::type: {
            auto resp_msg = dynamic_cast<delete_character_response const*>(msg);

            if(!resp_msg) {
                return;
            }

            _characters.erase(remove_if(begin(_characters), end(_characters), [slot = resp_msg->slot](const character_object &c) noexcept { return c.slot == slot; }), end(_characters));
            _waiting_for_reply = false;
            _show_delete = false;
            break;
        }
        case play_character_response::type: {
            auto resp_msg = dynamic_cast<play_character_response const*>(msg);

            if(!resp_msg) {
                return;
            }

            if(resp_msg->slot == static_cast<uint32_t>(_selected_play_slot)) {
                manager->get_character() = *std::find_if(begin(_characters), end(_characters), [slot = _selected_play_slot](auto const &c){ return c.slot == static_cast<uint32_t>(slot); });
                manager->add(make_unique<battle_log_scene>());
                _waiting_for_reply = false;
                _closed = true;
            } else {
                spdlog::error("[{}] selected character slot {} does not match server's idea of slot {}", __FUNCTION__, _selected_play_slot, resp_msg->slot);
            }
            break;
        }
        case generic_error_response::type: {
            auto resp_msg = dynamic_cast<generic_error_response const*>(msg);

            if(!resp_msg) {
                return;
            }

            _waiting_for_select = false;
            _waiting_for_reply = false;
            _error = resp_msg->error;
            break;
        }
        default:
            break;
    }
}
