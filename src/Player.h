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

#pragma once

#include "PlayerRole.h"

#include <User.h>
#include <Json.h>

#include <functional>
#include <vector>

class WerewolfDriver;

class Player
{
public:
	using Role = PlayerRole;

	Player(WerewolfDriver *driver, KA_IMPORT User *user);
	~Player();

	KA_IMPORT uint uid() const;
	KA_IMPORT User *user() const;

	Role role() const;
	void setRole(Role role);

	Role initialRole() const;
	void setInitialRole(Role role);

	void deliverRoleCard();

	void showPlayerRole(Player *target);
	void showExtraCard(KA_IMPORT uint id, PlayerRole role);

	KA_IMPORT Json fetchReply() const;

	std::vector<Player *> fetchChosenPlayers() const;
	Player *fetchChosenPlayer(Player *except = nullptr) const;
	std::vector<Player *> fetchChosenPlayers(int num, Player *except = nullptr) const;

	std::vector<int> fetchChosenCards() const;
	int fetchChosenCard() const;
	std::vector<int> fetchChosenCards(int num) const;

	using Callback = std::function<void(const KA_IMPORT Json &)>;
	void one(int command, const Callback &callback);
	void fire(int command, const KA_IMPORT Json &args);

private:
	KA_DECLARE_PRIVATE
};
