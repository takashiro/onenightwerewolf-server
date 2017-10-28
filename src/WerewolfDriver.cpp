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

#include <vector>
#include <thread>
#include <algorithm>

struct WerewolfDriver::Private
{
	std::vector<Player *> players;
	PlayerRole extraCards[3];
};

WerewolfDriver::WerewolfDriver()
	: d(new Private)
{
}

WerewolfDriver::~WerewolfDriver()
{
	delete d;
}

void WerewolfDriver::start()
{
	room()->broadcastNotification(cmd::StartGame);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	std::vector<PlayerRole> roles(d->players.size() + 3);
	for (int i = 0; i < roles.size(); i++) {
		roles[i] = static_cast<PlayerRole>(i);
	}

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
