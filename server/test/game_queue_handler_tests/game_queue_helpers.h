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
#include <game_queue_messages/messages.h>

namespace ibh {
    template <class T>
    void test_outmsg(outward_queues &q, bool should_be_empty) {
        outward_message outmsg(0, nullptr);
        REQUIRE(q.try_dequeue_from_producer(outmsg) == true);
        REQUIRE(outmsg.msg);
        auto *outmsgptr = dynamic_cast<T*>(outmsg.msg.get());
        REQUIRE(outmsgptr != nullptr);
        REQUIRE(outmsgptr->error.empty() == should_be_empty);
    }
}