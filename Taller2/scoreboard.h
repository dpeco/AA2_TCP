#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <SFML\Network.hpp>

struct Player {
	std::string name = "tempname";
	bool ready = false;
	int turn; //turno de los jugadores
	int score = 0;
	sf::TcpSocket* socket; //o algo del estilo para poder saber su port e identificarlo en la lista de clientes
};

class ScoreBoard {

	std::vector<Player> players;
public:
	ScoreBoard() {};
	~ScoreBoard() {};

	void UpdatePlayer(Player player); //a�ade/actualiza jugador
};