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

#include <vector>

class Player;
class WerewolfDriver;

class PlayerAction
{
public:
	PlayerAction(PlayerRole role, int priority);
	virtual ~PlayerAction() {}

	PlayerRole role() const { return mRole; }
	int priority() const { return mPriority; }

	virtual void start(WerewolfDriver *driver) const;

	virtual bool isEffective(Player *player) const;
	virtual void takeEffect(WerewolfDriver *driver, Player *player) const = 0;

	virtual void end(WerewolfDriver *driver) const;

private:
	PlayerRole mRole;
	int mPriority;
};

std::vector<PlayerAction *> CreatePlayerActions(const std::vector<PlayerRole> &roles);
