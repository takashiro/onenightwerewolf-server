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

KA_USING_NAMESPACE

struct Player::Private
{
	User *user;
	PlayerRole role;
	PlayerRole initialRole;
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

Player::Role Player::initialRole() const
{
	return d->initialRole;
}

void Player::setInitialRole(Role role)
{
	d->initialRole = role;
}

void Player::deliverRoleCard()
{
	if (d->user) {
		int role = static_cast<int>(d->role);
		d->user->notify(cmd::DeliverRoleCard, role);
	}
}

void Player::showPlayerRole(Player *target)
{
	JsonObject args;
	args["uid"] = target->user()->id();
	args["role"] = static_cast<int>(target->role());
	d->user->notify(cmd::ShowPlayerRole, args);
}

void Player::showExtraCard(KA_IMPORT uint id, PlayerRole role)
{
	JsonObject info;
	info["id"] = id;
	info["role"] = static_cast<int>(role);
	d->user->notify(cmd::ShowExtraCard, info);
}

KA_IMPORT Json Player::getReply() const
{
	return d->user->getReply();
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
