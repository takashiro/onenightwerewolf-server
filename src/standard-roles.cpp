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

	void takeEffect(WerewolfDriver *driver, Player *player) const override
	{
		Json answer = player->getReply();
		if (!answer.isArray()) {
			return;
		}

		JsonArray selected_players = answer.toArray();
		if (selected_players.empty()) {
			return;
		}

		uint chosen_id = selected_players[0].toUInt();
		Player *target = driver->findPlayer(chosen_id);
		if (target) {
			player->setRole(target->role());
			player->showPlayerRole(target);
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

	void takeEffect(WerewolfDriver *driver, Player *player) const override
	{
		std::vector<Player *> wolves = driver->findPlayers(PlayerRole::Werewolf);
		for (Player *wolf : wolves) {
			if (wolf == player) {
				continue;
			}
			player->showPlayerRole(wolf);
		}

		if (wolves.size() == 1) {
			Json answer = player->getReply();
			uint index = answer.toUInt();
			if (index > 2) {
				index = 2;
			}

			const PlayerRole *extra_cards = driver->extraCards();
			player->showExtraCard(index, extra_cards[index]);
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
		Json answer = seer->getReply();
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

	void takeEffect(WerewolfDriver *driver, Player *robber) const override
	{
		Json answer = robber->getReply();
		if (!answer.isArray()) {
			return;
		}

		JsonArray selected_players = answer.toArray();
		if (selected_players.empty()) {
			return;
		}

		uint chosen_id = selected_players[0].toUInt();
		Player *target = driver->findPlayer(chosen_id);
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

	void takeEffect(WerewolfDriver *driver, Player *trouble_maker) const override
	{
		Json answer = trouble_maker->getReply();
		if (!answer.isArray()) {
			return;
		}

		const JsonArray &targets = answer.toArray();
		JsonArray::const_iterator iter = targets.begin();
		Player *target[2] = {nullptr};
		for (int i = 0; i < 2; i++) {
			target[i] = driver->findPlayer((*iter).toUInt());
			iter++;
		}

		if (target[0] == nullptr || target[1] == nullptr) {
			return;
		}

		PlayerRole tmp = target[0]->role();
		target[0]->setRole(target[1]->role());
		target[1]->setRole(tmp);
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
		Json answer = drunk->getReply();
		if (!answer.isArray()) {
			return;
		}

		JsonArray selected_cards = answer.toArray();
		if (selected_cards.empty()) {
			return;
		}

		uint card_id = selected_cards[0].toUInt();
		if (card_id < 3) {
			PlayerRole *cards = driver->extraCards();
			PlayerRole old_role = drunk->role();
			drunk->setRole(cards[card_id]);
			cards[card_id] = old_role;
		}
	}
};

class Insomniac : public PlayerAction
{
public:
	Insomniac()
		: PlayerAction(PlayerRole::Insomniac, 8)
	{
	}

	void takeEffect(WerewolfDriver *driver, Player *insomniac) const override
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

	std::vector<PlayerAction *> actions;
	for (PlayerRole role : roles) {
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
