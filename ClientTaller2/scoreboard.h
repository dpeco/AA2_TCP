#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
struct Player {
	std::string name;
	int score;
};
class ScoreBoard {

	std::vector<Player> players;
public:
	ScoreBoard() {};
	~ScoreBoard() {};

	void PrintBoard(); //printea scoreboard
	void UpdatePlayer(Player player); //añade/actualiza jugador
};