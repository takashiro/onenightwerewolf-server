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

#include "Player.h"

#include "cmd.h"

#include <Json.h>
#include <User.h>

#include <map>

struct Player::Private
{
	KA_IMPORT User *user;
	PlayerRole role;
	std::multimap<int, Player::Callback> callbacks;

	Private()
		: user(nullptr)
		, role(PlayerRole::Unknown)
	{
	}
};

Player::Player(KA_IMPORT User *user)
	: d(new Private)
{
	d->user = user;
}

Player::~Player()
{
	delete d;
}

KA_IMPORT User *Player::user() const
{
	return d->user;
}

Player::Role Player::role() const
{
	return d->role;
}

void Player::setRole(Role role)
{
	d->role = role;
}

void Player::deliverRoleCard()
{
	if (d->user) {
		int role = static_cast<int>(d->role);
		d->user->notify(cmd::DeliverRoleCard, role);
	}
}

void Player::one(int command, const Callback &callback)
{
	d->callbacks.insert(std::pair<int, const Callback &>(command, callback));
}

void Player::fire(int command, const KA_IMPORT Json &args)
{
	auto range = d->callbacks.equal_range(command);
	for (auto i = range.first; i != range.second; i++) {
		i->second(args);
	}

	d->callbacks.erase(command);
}
