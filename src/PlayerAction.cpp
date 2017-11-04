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
#include "Player.h"

PlayerAction::PlayerAction(PlayerRole role, int priority)
	: mRole(role)
	, mPriority(priority)
{
}

void PlayerAction::start(WerewolfDriver *driver) const
{
}

bool PlayerAction::isEffective(Player *player) const
{
	return mRole == player->initialRole();
}

void PlayerAction::end(WerewolfDriver *driver) const
{
}
