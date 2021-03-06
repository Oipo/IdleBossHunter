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


#include "set_motd_handler.h"

#include <spdlog/spdlog.h>

#include <messages/moderator/set_motd_request.h>
#include <websocket_thread.h>
#include <messages/moderator/update_motd_response.h>
#include "message_handlers/handler_macros.h"
#include "macros.h"

#ifdef TEST_CODE
#include "../../../test/custom_server.h"
#endif

using namespace std;

namespace ibh {
    template <class Server, class WebSocket>
    void handle_set_motd(Server *s, rapidjson::Document const &d, unique_ptr<database_transaction> const &transaction, per_socket_data<WebSocket> *user_data,
                          queue_abstraction<unique_ptr<queue_message>> *q, ibh_flat_map<uint64_t, per_socket_data<WebSocket>> &user_connections) {
        if(!user_data->is_game_master) {
            spdlog::warn("[{}] user {} tried to set motd but is not a game master!", __FUNCTION__, user_data->username);
            return;
        }

        MEASURE_TIME_OF_FUNCTION(trace);
        DESERIALIZE_WITH_PLAYING_CHECK(set_motd_request);

        spdlog::info("[{}] motd set to \"{}\" by user {}", __FUNCTION__, msg->motd, user_data->username);
        motd = msg->motd;

        update_motd_response motd_msg(motd);
        auto motd_msg_str = motd_msg.serialize();
        {
            shared_lock lock(user_connections_mutex);
            for (auto &[conn_id, other_user_data] : user_connections) {
                try {
                    if (other_user_data.ws.expired()) {
                        continue;
                    }
                    s->send(other_user_data.ws, motd_msg_str, websocketpp::frame::opcode::value::TEXT);
                } catch (...) {
                    continue;
                }
            }
        }
    }

    template void handle_set_motd<server, websocketpp::connection_hdl>(server *s, rapidjson::Document const &d, unique_ptr<database_transaction> const &transaction,
                                                                        per_socket_data<websocketpp::connection_hdl> *user_data,
                                                                        queue_abstraction<unique_ptr<queue_message>> *q, ibh_flat_map<uint64_t, per_socket_data<websocketpp::connection_hdl>> &user_connections);

#ifdef TEST_CODE
    template void handle_set_motd<custom_server, custom_hdl>(custom_server *s, rapidjson::Document const &d, unique_ptr<database_transaction> const &transaction,
                                                           per_socket_data<custom_hdl> *user_data, queue_abstraction<unique_ptr<queue_message>> *q, ibh_flat_map<uint64_t, per_socket_data<custom_hdl>> &user_connections);
#endif
}
