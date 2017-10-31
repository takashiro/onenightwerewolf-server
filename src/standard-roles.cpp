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

#include "PlayerAction.h"

#include "cmd.h"
#include "Player.h"
#include "PlayerRole.h"
#include "WerewolfDriver.h"

#include <Json.h>
#include <Room.h>

KA_USING_NAMESPACE

class Doppelganger : public PlayerAction
{
public:
	Doppelganger()
		: PlayerAction(PlayerRole::Doppelganger, 0)
	{
	}

	void takeEffect(WerewolfDriver *driver, Player *player) const override
	{
		player->one(cmd::ChoosePlayer, [=] (const Json &args) {
			uint chosen_id = args.toUInt();
			Player *target = driver->findPlayer(chosen_id);
			if (target) {
				player->setRole(target->role());

				User *user = player->user();
				if (user) {
					JsonObject args;
					args["uid"] = chosen_id;
					args["role"] = static_cast<int>(target->role());
					user->notify(cmd::ShowPlayerRole, args);
				}
			}
		});

		Room *room = driver->room();
		room->broadcastNotification(cmd::ChoosePlayer, 1);
	}
};

std::vector<PlayerAction *> CreatePlayerActions()
{
	std::vector<PlayerAction *> actions;
	actions.push_back(new Doppelganger);
	return actions;
}
