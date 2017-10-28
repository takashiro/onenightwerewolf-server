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

#include <GameDriver.h>

class WerewolfDriver : public KA_IMPORT GameDriver
{
public:
	WerewolfDriver();
	~WerewolfDriver();

	void start() override;
	void end() override;

	void addPlayer(KA_IMPORT User *user) override;
	void removePlayer(KA_IMPORT User *user) override;

private:
	KA_DECLARE_PRIVATE
};
