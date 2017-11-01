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
#include <Room.h>

#include <thread>
#include <chrono>

KA_USING_NAMESPACE

class Doppelganger : public PlayerAction
{
public:
	Doppelganger()
		: PlayerAction(PlayerRole::Doppelganger, 0)
	{
	}

	void takeEffect(WerewolfDriver *driver, Player *player) const override
	{
		player->one(cmd::ChoosePlayer, [=] (const Json &args) {
			uint chosen_id = args.toUInt();
			Player *target = driver->findPlayer(chosen_id);
			if (target) {
				player->setRole(target->role());
				player->showPlayerRole(target);
			}
		});

		Room *room = driver->room();
		room->broadcastNotification(cmd::ChoosePlayer, 1);
	}
};

class Werewolf : public PlayerAction
{
public:
	Werewolf()
		: PlayerAction(PlayerRole::Werewolf, 1)
	{
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

	void takeEffect(WerewolfDriver *driver, Player *seer) const override
	{
		seer->one(cmd::ChoosePlayerOrCard, [=] (const Json &args) {
			if (!args.isObject()) {
				return;
			}

			bool choose_player = true;
			const JsonObject &input = args.toObject();
			auto i = input.find("type");
			if (i != input.end() && i->second.toString() == "card") {
				choose_player = false;
			}

			auto j = input.find("targets");
			if (j != input.end() && i->second.isArray()) {
				const JsonArray &chosen = i->second.toArray();
				if (chosen.empty()) {
					return;
				}

				if (choose_player) {
					Player *target = driver->findPlayer(chosen.front().toUInt());
					if (target) {
						seer->showPlayerRole(target);
					}
				} else {
					const PlayerRole *extra_cards = driver->extraCards();
					for (const Json &target : chosen) {
						uint id = target.toUInt();
						if (id < 3) {
							seer->showExtraCard(id, extra_cards[id]);
						}
					}
				}
				std::this_thread::sleep_for(std::chrono::seconds(3));
			}
		});
	}
};

class Robber : public PlayerAction
{
public:
	Robber()
		: PlayerAction(PlayerRole::Robber, 5)
	{
	}

	void takeEffect(WerewolfDriver *driver, Player *robber) const override
	{
		robber->one(cmd::ChoosePlayer, [=] (const Json &args) {
			uint chosen_id = args.toUInt();
			Player *target = driver->findPlayer(chosen_id);
			if (target) {
				robber->showPlayerRole(target);
				robber->setRole(target->role());
				target->setRole(PlayerRole::Robber);
			}
		});

		Room *room = driver->room();
		room->broadcastNotification(cmd::ChoosePlayer, 1);
	}
};

std::vector<PlayerAction *> CreatePlayerActions()
{
	std::vector<PlayerAction *> actions;
	actions.push_back(new Doppelganger);
	actions.push_back(new Werewolf);
	actions.push_back(new Minion);
	actions.push_back(new Mason);
	actions.push_back(new Seer);
	actions.push_back(new Robber);
	return actions;
}
