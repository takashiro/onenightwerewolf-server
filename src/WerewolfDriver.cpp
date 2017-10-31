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

#include "WerewolfDriver.h"

#include "cmd.h"
#include "Player.h"
#include "PlayerRole.h"

#include <Room.h>
#include <Json.h>

#include <vector>
#include <thread>
#include <algorithm>

KA_USING_NAMESPACE

struct WerewolfDriver::Private
{
	Json config;
	std::vector<PlayerRole> roles;
	std::vector<Player *> players;
	PlayerRole extraCards[3];

	void updateConfig()
	{
		JsonObject config;

		JsonArray roles;
		for (PlayerRole role : this->roles) {
			roles.push_back(static_cast<int>(role));
		}
		config["roles"] = roles;

		this->config = std::move(config);
	}
};

WerewolfDriver::WerewolfDriver()
	: d(new Private)
{
}

WerewolfDriver::~WerewolfDriver()
{
	delete d;
}

void WerewolfDriver::setConfig(const KA_IMPORT Json &config)
{
	if (!config.isObject()) {
		return;
	}

	JsonObject setting = config.toObject();
	if (setting.find("roles") != setting.end()) {
		Json role_list = setting.at("roles");
		if (role_list.isArray()) {
			JsonArray roles = role_list.toArray();
			d->roles.clear();
			int max_value = static_cast<int>(PlayerRole::MaxLimit);
			for (const Json &role : roles) {
				int role_value = role.toInt();
				if (role_value < max_value) {
					d->roles.push_back(static_cast<PlayerRole>(role_value));
				}
			}
		}
	}

	d->updateConfig();
}

const Json &WerewolfDriver::config() const
{
	return d->config;
}

void WerewolfDriver::run()
{
	std::this_thread::sleep_for(std::chrono::seconds(1));

	std::vector<PlayerRole> roles = d->roles;

	// Arrange roles
	std::random_shuffle(roles.begin(), roles.end());
	int i = 0;
	for (Player *player : d->players) {
		player->setRole(roles[i]);
		i++;
	}
	for (int j = 0; j < 3; j++, i++) {
		d->extraCards[j] = roles[i];
	}

	// Notify roles
	for (Player *player : d->players) {
		player->deliverRoleCard();
	}
}

void WerewolfDriver::end()
{
}

void WerewolfDriver::addPlayer(KA_IMPORT User *user)
{
	d->players.push_back(new Player(user));
}

void WerewolfDriver::removePlayer(KA_IMPORT User *user)
{
	d->players.erase(std::find_if(d->players.begin(), d->players.end(), [=] (Player *player) {
		return player->user() == user;
	}));
}

void WerewolfDriver::setRoles(std::vector<PlayerRole> &&roles)
{
	d->roles = std::move(roles);
}

const std::vector<PlayerRole> &WerewolfDriver::roles() const
{
	return d->roles;
}
