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

#include <algorithm>
#include <chrono>
#include <map>
#include <random>
#include <set>
#include <thread>

KA_USING_NAMESPACE

class Doppelganger : public PlayerAction
{
public:
	Doppelganger()
		: PlayerAction(PlayerRole::Doppelganger, 0)
	{
	}

	void start(WerewolfDriver *driver) const override
	{
		driver->broadcastToChoosePlayer(1);
	}

	void takeEffect(WerewolfDriver *, Player *doppelganger) const override
	{
		Player *target = doppelganger->fetchChosenPlayer(doppelganger);
		if (target) {
			doppelganger->showPlayerRole(target);
			doppelganger->setRole(target->role());
			doppelganger->showPlayerRole(doppelganger);
		}
	}
};

class Werewolf : public PlayerAction
{
public:
	Werewolf()
		: PlayerAction(PlayerRole::Werewolf, 1)
	{
	}

	void start(WerewolfDriver *driver) const override
	{
		driver->broadcastToChooseCard(1);
	}

	void takeEffect(WerewolfDriver *driver, Player *werewolf) const override
	{
		std::vector<Player *> wolves = driver->findPlayers(PlayerRole::Werewolf);
		for (Player *wolf : wolves) {
			if (wolf == werewolf) {
				continue;
			}
			werewolf->showPlayerRole(wolf);
		}

		if (wolves.size() == 1) {
			int index = werewolf->fetchChosenCard();
			const PlayerRole *extra_cards = driver->extraCards();
			werewolf->showExtraCard(index, extra_cards[index]);
		}
	}

	void end(WerewolfDriver *) const override
	{
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
};

class Minion : public PlayerAction
{
public:
	Minion()
		: PlayerAction(PlayerRole::Minion, 2)
	{
	}

	void takeEffect(WerewolfDriver *driver, Player *player) const override
	{
		std::vector<Player *> wolves = driver->findPlayers(PlayerRole::Werewolf);
		for (Player *wolf : wolves) {
			player->showPlayerRole(wolf);
		}
	}

	void end(WerewolfDriver *) const override
	{
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
};

class Mason : public PlayerAction
{
public:
	Mason()
		: PlayerAction(PlayerRole::Mason, 3)
	{
	}

	void takeEffect(WerewolfDriver *driver, Player *player) const override
	{
		std::vector<Player *> masons = driver->findPlayers(PlayerRole::Mason);
		for (Player *mason : masons) {
			if (mason == player) {
				continue;
			}
			player->showPlayerRole(mason);
		}
	}
};

class Seer : public PlayerAction
{
public:
	Seer()
		: PlayerAction(PlayerRole::Seer, 4)
	{
	}

	void start(WerewolfDriver *driver) const override
	{
		driver->broadcastToChoosePlayerOrCard(1, 2);
	}

	void takeEffect(WerewolfDriver *driver, Player *seer) const override
	{
		Json answer = seer->fetchReply();
		if (!answer.isObject()) {
			return;
		}

		bool choose_player = true;
		const JsonObject &input = answer.toObject();
		auto i = input.find("type");
		if (i != input.end() && i->second.toString() == "card") {
			choose_player = false;
		}

		auto j = input.find("targets");
		if (j != input.end() && j->second.isArray()) {
			const JsonArray &chosen = j->second.toArray();
			if (chosen.empty()) {
				return;
			}

			if (choose_player) {
				Player *target = driver->findPlayer(chosen.front().toUInt());
				if (target) {
					seer->showPlayerRole(target);
				}
			} else {
				if (chosen.size() < 2) {
					return;
				}

				const PlayerRole *extra_cards = driver->extraCards();
				for (int i = 0; i < 2; i++) {
					const Json &target = chosen[i];
					uint id = target.toUInt();
					if (id < 3) {
						seer->showExtraCard(id, extra_cards[id]);
					}
				}
			}
		}
	}

	void end(WerewolfDriver *) const override
	{
		std::this_thread::sleep_for(std::chrono::seconds(3));
	}
};

class Robber : public PlayerAction
{
public:
	Robber()
		: PlayerAction(PlayerRole::Robber, 5)
	{
	}

	void start(WerewolfDriver *driver) const override
	{
		driver->broadcastToChoosePlayer(1);
	}

	void takeEffect(WerewolfDriver *, Player *robber) const override
	{
		Player *target = robber->fetchChosenPlayer(robber);
		if (target) {
			robber->showPlayerRole(target);
			robber->setRole(target->role());
			target->setRole(PlayerRole::Robber);
		}
	}
};

class TroubleMaker : public PlayerAction
{
public:
	TroubleMaker()
		: PlayerAction(PlayerRole::TroubleMaker, 6)
	{
	}

	void start(WerewolfDriver *driver) const override
	{
		driver->broadcastToChoosePlayer(2);
	}

	void takeEffect(WerewolfDriver *, Player *trouble_maker) const override
	{
		std::vector<Player *> targets = trouble_maker->fetchChosenPlayers(2, trouble_maker);
		if (targets.size() > 2) {
			PlayerRole tmp = targets[0]->role();
			targets[0]->setRole(targets[1]->role());
			targets[1]->setRole(tmp);
		}
	}
};

class Drunk : public PlayerAction
{
public:
	Drunk()
		: PlayerAction(PlayerRole::Drunk, 7)
	{
	}

	void start(WerewolfDriver *driver) const override
	{
		driver->broadcastToChooseCard(1);
	}

	void takeEffect(WerewolfDriver *driver, Player *drunk) const override
	{
		int index = drunk->fetchChosenCard();
		PlayerRole *cards = driver->extraCards();
		PlayerRole old_role = drunk->role();
		drunk->setRole(cards[index]);
		cards[index] = old_role;
	}
};

class Insomniac : public PlayerAction
{
public:
	Insomniac()
		: PlayerAction(PlayerRole::Insomniac, 8)
	{
	}

	void takeEffect(WerewolfDriver *, Player *insomniac) const override
	{
		insomniac->showPlayerRole(insomniac);
	}
};

using ActionMap = std::map<PlayerRole, PlayerAction *(*)()>;

#define ONW_ADD_ACTION(action) actions[PlayerRole::action] = [] () -> PlayerAction * { return new action; }

static ActionMap CreateActionMap()
{
	ActionMap actions;
	ONW_ADD_ACTION(Doppelganger);
	ONW_ADD_ACTION(Werewolf);
	ONW_ADD_ACTION(Minion);
	ONW_ADD_ACTION(Mason);
	ONW_ADD_ACTION(Seer);
	ONW_ADD_ACTION(Robber);
	ONW_ADD_ACTION(TroubleMaker);
	ONW_ADD_ACTION(Drunk);
	ONW_ADD_ACTION(Insomniac);
	return actions;
}

#undef ONW_ADD_ACTION

std::vector<PlayerAction *> CreatePlayerActions(const std::vector<PlayerRole> &roles)
{
	static ActionMap action_map = CreateActionMap();

	std::set<PlayerRole> role_set;
	for (PlayerRole role : roles) {
		role_set.insert(role);
	}

	std::vector<PlayerAction *> actions;
	for (PlayerRole role : role_set) {
		ActionMap::const_iterator iter = action_map.find(role);
		if (iter != action_map.end()) {
			PlayerAction *action = (iter->second)();
			if (action) {
				actions.push_back(action);
			}
		}
	}
	return actions;
}
