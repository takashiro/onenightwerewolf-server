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
#include "WerewolfDriver.h"

#include <Json.h>
#include <User.h>

#include <algorithm>
#include <map>
#include <random>

KA_USING_NAMESPACE

struct Player::Private
{
	WerewolfDriver *driver;
	User *user;
	PlayerRole role;
	PlayerRole initialRole;
	std::multimap<int, Player::Callback> callbacks;

	Private()
		: driver(nullptr)
		, user(nullptr)
		, role(PlayerRole::Unknown)
	{
	}
};

Player::Player(WerewolfDriver *driver, KA_IMPORT User *user)
	: d(new Private)
{
	d->driver = driver;
	d->user = user;
}

Player::~Player()
{
	delete d;
}

KA_IMPORT uint Player::uid() const
{
	return d->user ? d->user->id() : 0;
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
	showPlayerRole(this);
}

void Player::showPlayerRole(Player *target)
{
	User *user = target->user();
	if (user) {
		JsonObject args;
		args["uid"] = user->id();
		args["role"] = static_cast<int>(target->role());
		d->user->notify(cmd::ShowPlayerRole, args);
	}
}

void Player::showExtraCard(KA_IMPORT uint id, PlayerRole role)
{
	JsonObject info;
	info["id"] = id;
	info["role"] = static_cast<int>(role);
	d->user->notify(cmd::ShowExtraCard, info);
}

KA_IMPORT Json Player::fetchReply() const
{
	return d->user->fetchReply();
}

std::vector<Player *> Player::fetchChosenPlayers() const
{
	std::vector<Player *> targets;

	Json answer = this->fetchReply();
	if (!answer.isArray()) {
		return targets;
	}

	JsonArray selected_players = answer.toArray();
	if (selected_players.empty()) {
		return targets;
	}

	for (const Json &selected : selected_players) {
		uint chosen_id = selected.toUInt();
		Player *target = d->driver->findPlayer(chosen_id);
		if (target) {
			targets.push_back(target);
		}
	}

	return targets;
}

Player *Player::fetchChosenPlayer(Player *except) const
{
	std::vector<Player *> targets = this->fetchChosenPlayers();
	if (!targets.empty() && targets[0] != except) {
		return targets[0];
	}

	const std::vector<Player *> &players = d->driver->players();
	if (players.empty()) {
		return nullptr;
	}

	if (players.size() == 1) {
		return players[0] != except ? players[0] : nullptr;
	}

	std::random_device rd;
	std::mt19937 g(rd());
	for (;;) {
		uint index = g() % players.size();
		Player *target = players[index];
		if (target != except) {
			return target;
		}
	}
}

std::vector<Player *> Player::fetchChosenPlayers(int num, Player *except) const
{
	std::vector<Player *> targets = this->fetchChosenPlayers();
	if (except) {
		auto iter = std::find(targets.begin(), targets.end(), except);
		if (iter != targets.end()) {
			targets.erase(iter);
		}
	}
	if (targets.size() >= num) {
		return targets;
	}

	std::vector<Player *> others = d->driver->players();
	if (except) {
		others.erase(std::find(others.begin(), others.end(), except));
	}

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(others.begin(), others.end(), g);
	if (others.size() > num) {
		others.resize(num);
	}

	return others;
}

std::vector<int> Player::fetchChosenCards() const
{
	std::vector<int> cards;

	Json answer = this->fetchReply();
	if (!answer.isArray()) {
		return cards;
	}

	JsonArray selected_cards = answer.toArray();
	if (selected_cards.empty()) {
		return cards;
	}

	for (const Json &selected : selected_cards) {
		int index = selected.toInt();
		if (0 <= index && index < d->driver->extraCardNum()) {
			cards.push_back(index);
		}
	}

	return cards;
}

int Player::fetchChosenCard() const
{
	std::vector<int> cards = this->fetchChosenCards();
	if (!cards.empty()) {
		return cards[0];
	}

	std::random_device rd;
	std::mt19937 g(rd());
	return static_cast<int>(g() % d->driver->extraCardNum());
}

std::vector<int> Player::fetchChosenCards(int num) const
{
	std::vector<int> cards = this->fetchChosenCards();
	if (cards.size() >= num) {
		return cards;
	}

	std::random_device rd;
	std::mt19937 g(rd());
	while (cards.size() < num) {
		cards.push_back(static_cast<int>(g() % d->driver->extraCardNum()));
	}

	return cards;
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
