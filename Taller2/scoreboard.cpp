#include "scoreboard.h"

bool cmpFunc(Player a, Player b) { return a.score > b.score; }

void ScoreBoard::UpdatePlayer(Player player) {
	bool playerFound = false;
	if (players.size() > 0) {
		//buscar si el nombre ya esta usado y actualizar puntos. Si no, añadirlo
		for (int i = 0; i < players.size(); i++) {
			if (strcmp(player.name.c_str(), players[i].name.c_str()) == 0) {
				players[i].score = player.score;
				playerFound = true;
				break;
			}
		}
	}

	if (!playerFound) {
		players.push_back(player); //añadir player si es nuevo
	}

	//ordenar
	std::sort(players.begin(), players.end(), cmpFunc);
}
void ScoreBoard::DeletePlayer(Player player) {
	bool playerFound = false;
	//buscar si el nombre ya esta usado y borrarlo
	for (int i = 0; i < players.size(); i++) {
		if (strcmp(player.name.c_str(), players[i].name.c_str()) == 0) {
			players.erase(players.begin() + i);
			playerFound = true;
			break;
		}
	}
	
	if (!playerFound) {
		players.push_back(player); //añadir player si es nuevo
	}

	//ordenar
	std::sort(players.begin(), players.end(), cmpFunc);
}
std::string ScoreBoard::Winner() {
	return players[0].name;
}