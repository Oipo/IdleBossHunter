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

#include "users_repository.h"
#include <spdlog/spdlog.h>

using namespace ibh;

template class ibh::users_repository<database_transaction>;
template class ibh::users_repository<database_subtransaction>;

template<DatabaseTransaction transaction_T>
bool users_repository<transaction_T>::insert_if_not_exists(db_user &usr, unique_ptr<transaction_T> const &transaction) const {
    auto result = transaction->execute(fmt::format(
            "INSERT INTO users (username, password, email, login_attempts, verification_code, is_game_master, max_characters) VALUES ('{}', '{}', '{}', {}, '{}', {}, {}) ON CONFLICT DO NOTHING RETURNING id",
            transaction->escape(usr.username), transaction->escape(usr.password), transaction->escape(usr.email), usr.login_attempts, transaction->escape(usr.verification_code), usr.is_game_master, usr.max_characters));

    spdlog::trace("[{}] contains {} entries", __FUNCTION__, result.size());

    if(result.empty()) {
        //already exists
        return false;
    }

    usr.id = result[0][0].as(uint64_t{});

    return true;
}

template<DatabaseTransaction transaction_T>
void users_repository<transaction_T>::update(db_user const &usr, unique_ptr<transaction_T> const &transaction) const {
    auto result = transaction->execute(fmt::format("UPDATE users SET username = '{}', password = '{}', email = '{}', login_attempts = {}, verification_code = '{}', is_game_master = {}, max_characters = {} WHERE id = {}",
                                                   transaction->escape(usr.username), transaction->escape(usr.password), transaction->escape(usr.email), usr.login_attempts, transaction->escape(usr.verification_code), usr.is_game_master, usr.max_characters, usr.id));

    spdlog::trace("[{}] contains {} entries", __FUNCTION__, result.size());
}

template<DatabaseTransaction transaction_T>
optional<db_user> users_repository<transaction_T>::get(int id, unique_ptr<transaction_T> const &transaction) const {
    auto result = transaction->execute(fmt::format("SELECT * FROM users WHERE id = {}", id));

    spdlog::trace("[{}] contains {} entries", __FUNCTION__, result.size());

    if(result.empty()) {
        return {};
    }

    return make_optional<db_user>(result[0]["id"].as(uint64_t{}), result[0]["username"].as(string{}),
                                  result[0]["password"].as(string{}), result[0]["email"].as(string{}),
                                  result[0]["login_attempts"].as(uint16_t{}), result[0]["verification_code"].as(string{}),
                                  result[0]["max_characters"].as(uint16_t{}), result[0]["is_game_master"].as(uint16_t{}));
}

template<DatabaseTransaction transaction_T>
optional<db_user> users_repository<transaction_T>::get(string const &username, unique_ptr<transaction_T> const &transaction) const {
    auto result = transaction->execute(fmt::format("SELECT * FROM users WHERE username = '{}'", transaction->escape(username)));

    spdlog::trace("[{}] contains {} entries", __FUNCTION__, result.size());

    if(result.empty()) {
        return {};
    }

    return make_optional<db_user>(result[0]["id"].as(uint64_t{}), result[0]["username"].as(string{}),
                                  result[0]["password"].as(string{}), result[0]["email"].as(string{}),
                                  result[0]["login_attempts"].as(uint16_t{}), result[0]["verification_code"].as(string{}),
                                  result[0]["max_characters"].as(uint16_t{}), result[0]["is_game_master"].as(uint16_t{}));
}

template<DatabaseTransaction transaction_T>
vector<db_user> users_repository<transaction_T>::get_all(const unique_ptr<transaction_T> &transaction) const {
    pqxx::result result = transaction->execute(fmt::format("SELECT * FROM users u LEFT JOIN banned_users bu ON bu.user_id = u.id WHERE bu.id IS NULL"));

    spdlog::trace("[{}] contains {} entries", __FUNCTION__, result.size());

    vector<db_user> users;
    users.reserve(result.size());

    for(auto const & res : result) {
        users.emplace_back(res["id"].as(uint64_t{}), res["username"].as(string{}),
                           res["password"].as(string{}), res["email"].as(string{}),
                           res["login_attempts"].as(uint16_t{}), res["verification_code"].as(string{}),
                           res["max_characters"].as(uint16_t{}), res["is_game_master"].as(uint16_t{}));
    }

    return users;
}
