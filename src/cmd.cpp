/***********************************************************************
One Night Ultimate Werewolf
Copyright (C) 2017  Kazuichi Takashiro

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

takashiro@qq.com
************************************************************************/

#include "cmd.h"

#include "Player.h"
#include "WerewolfDriver.h"

#include <Room.h>
#include <User.h>
#include <UserAction.h>
#include <Json.h>

#include <map>

KA_USING_NAMESPACE

std::map<int, KA_IMPORT UserAction> CreateWerewolfActions()
{
	static std::map<int, UserAction> actions;

	actions[cmd::UpdatePlayer] = [] (User *user, const Json &arg) {
		if (!arg.isObject() || !arg.contains("info")) {
			return;
		}

		Room *room = user->room();
		if (room == nullptr) {
			return;
		}

		Json info = arg["info"];
		info["id"] = user->id();

		if (arg.contains("receiver")) {
			User *receiver = room->findUser(arg["receiver"].toInt());
			if (receiver) {
				receiver->notify(cmd::UpdatePlayer, info);
			}
		} else {
			room->broadcastNotification(cmd::UpdatePlayer, info, user);
		}
	};

	actions[cmd::EndGame] = [] (User *user, const Json &) {
		Room *room = user->room();
		if (room == nullptr || room->state() != Room::State::Stopped) {
			return;
		}

		WerewolfDriver *driver = dynamic_cast<WerewolfDriver *>(room->driver());
		if (driver == nullptr) {
			return;
		}

		JsonArray player_roles;
		const std::vector<Player *> &players = driver->players();
		for (Player *player : players) {
			JsonObject info;
			info["uid"] = player->uid();
			info["role"] = static_cast<int>(player->role());
			player_roles.push_back(std::move(info));
		}

		JsonArray unused_roles(3);
		const PlayerRole *extra_cards = driver->extraCards();
		for (int i = 0; i < 3; i++) {
			unused_roles[i] = static_cast<int>(extra_cards[i]);
		}

		JsonObject args;
		args["players"] = std::move(player_roles);
		args["extra_cards"] = std::move(unused_roles);
		room->broadcastNotification(cmd::EndGame, args);
	};

	return actions;
}

const std::map<int, KA_IMPORT UserAction> *WerewolfDriver::actions() const
{
	static std::map<int, UserAction> actions = CreateWerewolfActions();
	return &actions;
}
