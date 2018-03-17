#include <iostream>
#include <list>
#include "scoreboard.h"
enum commands { NOM, DEN, CON, INF, MSG, IMG, WRD, GUD, BAD, WNU, WIN, DIS, END, RNK, RDY, TIM };

std::vector<std::string> wordsVector{"coche", "robot", "camara", "pelota", "gafas", "libro", "piramide", "pistola", "gato", "caballo", "huevo", "gallina", "sombrero",
"pokemon", "mario", "sonic", "pacman", "tetris"};

Player* globalPlayerPtr = new Player;

int RemainingReady(std::vector<Player*> players) {
	int readyCount = players.size();
	int i = 0;
	for (int i = 0; i < players.size(); i++) {
		if (players[i]->ready) readyCount--;
	}
	return readyCount;
}
void DetectPlayer(sf::TcpSocket& client, std::vector<Player*> players) { //encuentra player segun socket, dado que hara falta en varias ocasiones creo que sera util
	//encontrar player comparando remoteport
	bool found = false;
	int i = 0;
	Player* player = new Player;
	while (!found && i < players.size()) {
		if (players[i]->socket->getRemotePort() == client.getRemotePort()) {
			globalPlayerPtr = players[i];
			found = true;
		}
		i++;
	}
}
void DetectPlayer(int turn, std::vector<Player*> players) { //encuentra player segun turno
	bool found = false;
	int i = 0;
	Player* player = new Player;
	while (!found && i < players.size()) {
		if (players[i]->turn == turn) {
			globalPlayerPtr = players[i];
			found = true;
		}
		i++;
	}
}
std::string PickWord() { //elige palabra random de la lista
	std::string wordPicked = "platano"; //default
	if (wordsVector.size() > 0) {
		int randomPick = rand() % wordsVector.size();
		wordPicked = wordsVector[randomPick];
		wordsVector.erase(wordsVector.begin() + randomPick); //asiegura que no se repitan palabras, cuando no hayan mas devolveria platano que es el default
	}
	return wordPicked;
}
void ControlServidor()
{
	bool running = true;
	bool gameStarted = false;
	bool startNewTurn = false;
	int curTurn = 0; //turno actual
	int playerNumber; //players.size() shortcut
	int maxTurns;
	bool checkWords = false;
	bool paintingPhase = false;
	std::string globalCurWord;
	ScoreBoard scoreboard;
	// Create a socket to listen to new connections
	sf::TcpListener listener;
	sf::Socket::Status status = listener.listen(50000);
	if (status != sf::Socket::Done)	{
		std::cout << "Error al abrir listener\n";
		exit(0);
	}
	// Create a list to store the future clients
	std::list<sf::TcpSocket*> clients;
	//vec of players
	std::vector<Player*> players;
	// Create a selector
	sf::SocketSelector selector;
	// Add the listener to the selector
	selector.add(listener);

	// Endless loop that waits for new connections
	while (running)	{
		
		// Make the selector wait for data on any socket
		if (selector.wait()) {
			// Test the listener
			if (selector.isReady(listener)) {
				// The listener is ready: there is a pending connection
				sf::TcpSocket* client = new sf::TcpSocket;
				if (listener.accept(*client) == sf::Socket::Done)
				{

					// Add the new client to the clients list
					std::cout << "Llega el cliente con puerto: " << client->getRemotePort() << std::endl;
					clients.push_back(client);
					//creamos new player
					Player* player = new Player;
					player->socket = client;
					players.push_back(player);

					// Add the new client to the selector so that we will
					// be notified when he sends something
					selector.add(*client);
				}
				else
				{
					// Error, we won't get a new connection, delete the socket
					std::cout << "Error al recoger conexión nueva\n";
					delete client;
				}
			}
			else {	//Receive:
				// The listener socket is not ready, test all other sockets (the clients)
				//lista de iteradores que iteran listas de tcp sockets
				std::list<std::list<sf::TcpSocket*>::iterator> itTemp; //si se desconecta un socket, lo guardamos en este it para borrarlo después
				for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it) {
					sf::TcpSocket& client = **it;
					if (selector.isReady(client)) {
						// The client has sent some data, we can receive it
						sf::Packet packet;
						status = client.receive(packet);

						if (status == sf::Socket::Done)	{

							int command;
							if (packet >> command) {//inicializa cosas antes del switch

								std::string strRec;
								std::string word;
								bool used = false;
								sf::Packet newPacket;
								sf::Packet imagePacket;
								int _w, _h;
								int arraySize;
								int remainingPlayers;
								DetectPlayer(client, players); //identifica al player
								bool sendWord = true; //envia mensaje o no
								switch (command) {
								case commands::MSG:
									packet >> strRec;
									std::cout << "He recibido " << strRec << " del puerto " << client.getRemotePort() << std::endl;

									//checkear si el msg es correcto
									if (checkWords) {
										std::string tempWord = " >" + globalCurWord;
										Player* wordPlayer = globalPlayerPtr; //nos guardamos quien esta escribiendo
										DetectPlayer(curTurn % playerNumber, players); //saber quien esta pintando
										if (strcmp(tempWord.c_str(), strRec.c_str()) == 0) { //comparar que sea la palabra correcta
											sendWord = false; //no enviar palabra correcta al chat
											//comprobar si ha sido dibujante o no
											if (wordPlayer->turn != globalPlayerPtr->turn && !wordPlayer->answered) { //asegurarse que no se repitan
												//gud al jugador, supongo que habria que calcular puntos
												wordPlayer->answered = true;
												wordPlayer->score += 1;
												scoreboard.UpdatePlayer(*wordPlayer);
												newPacket << commands::GUD;
												client.send(newPacket);
												for (std::list<sf::TcpSocket*>::iterator it2 = clients.begin(); it2 != clients.end(); ++it2) {
													sf::TcpSocket& tempSok = **it2;

													newPacket.clear();
													newPacket << commands::WIN << wordPlayer->name << wordPlayer->score;
													tempSok.send(newPacket);
												}
											}
										}
										else {
											//bad
											if (wordPlayer->turn != globalPlayerPtr->turn) { //comprobar que no sea al que ha pintado
												newPacket << commands::BAD;
												client.send(newPacket);
											}
										}
										
									}
									//Reenviar mensaje a todos los clientes:
									if (sendWord) {
										for (std::list<sf::TcpSocket*>::iterator it2 = clients.begin(); it2 != clients.end(); ++it2) {
											newPacket.clear();
											sf::TcpSocket& tempSok = **it2;

											newPacket << commands::MSG << globalPlayerPtr->name << strRec;
											tempSok.send(newPacket);
										}
									}
									
									break;
								case NOM:
									//compara con el resto de players si ya está usado o no
									packet >> strRec;
									for (int i = 0; i < players.size(); i++) {
										if (strcmp(players[i]->name.c_str(), strRec.c_str()) == 0) {
											used = true;
										}
									}
									//si no está usado --> send CON y añade, si lo está --> send DEN
									if (!used) {
										globalPlayerPtr->name = strRec;
										newPacket << commands::CON;
										client.send(newPacket);

										//avisamos a todos que se ha conectado un nuevo cliente
										for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it) {
											sf::TcpSocket& tempSok = **it;
											std::string playerStr = globalPlayerPtr->name;
											sf::Packet packet;
											packet << commands::INF << playerStr;
											tempSok.send(packet);
										}
									}
									else {
										newPacket << commands::DEN;
										client.send(newPacket);
									}
									break;
								case RDY:
									if (!gameStarted) {
										globalPlayerPtr->ready = true;
										remainingPlayers = RemainingReady(players);
										//avisamos a todos que el jugador está ready
										for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it) {
											sf::TcpSocket& tempSok = **it;
											sf::Packet packet;
											if (remainingPlayers > 0) {
												std::string playerStr = globalPlayerPtr->name;

												packet << commands::MSG << "EL JUGADOR " + playerStr + " ESTÁ PREPARADO PARA JUGAR (FALTA(n) " + std::to_string(remainingPlayers)
													+ " READY(s) PARA EMPEZAR)";
												tempSok.send(packet);
											}
											else if (players.size() == 1) {
												packet << commands::MSG << "FALTAN MÁS JUGADORES PARA PODER JUGAR";
												tempSok.send(packet);
											}
											else {
												packet << commands::MSG << "EMPIEZA LA PARTIDA";
												tempSok.send(packet);

												gameStarted = true;
											}
										}

										//ya que el juego va a empezar, aprovechamos para setear todo lo necesario y turno 1
										if (gameStarted) {
											curTurn = 0;
											playerNumber = players.size();
											maxTurns = playerNumber * 2;
											//crear orden de los turnos
											word = PickWord();
											globalCurWord = word;
											int sizeWord = word.size();
											for (int i = 0; i < playerNumber; i++) {
												players[i]->turn = i;
												if (i == 0) { sf::Packet turnPacket; turnPacket << commands::WRD << word;  players[i]->socket->send(turnPacket); }
												else { sf::Packet turnPacket; turnPacket << commands::WNU << players[0]->name << sizeWord;  players[i]->socket->send(turnPacket); }
												scoreboard.UpdatePlayer(*players[i]);
											}
										}
									}
									break;
								case IMG:
									/*
									recibe la imagen dibujada por el cliente que ha dibujado
										RECIBES Y REENVIAS EL MISMO PACKETE SIN MÁS
									quizá podria aprovechar y reenviar la imagen al resto, dependiendo de al final como va le code xd
									*/
									std::cout << "IMAGE RECEIVED" << std::endl;
									
									packet >> _w;
									packet >> _h;
									arraySize = _w * _h * 4;
									
									imagePacket << commands::IMG;
									imagePacket << _w << _h;
									for (int index = 0; index < arraySize; index++) {
										sf::Uint8 tempUint;
										packet >> tempUint;
										imagePacket << tempUint;
									}
									DetectPlayer(curTurn, players);	//Detect current player that is drawing.
									for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it) {
										sf::TcpSocket& tempSok = **it;
										
										if (globalPlayerPtr->socket->getRemotePort() != tempSok.getRemotePort()) {	//Only send image to everyone except for the drawer. Which is already drawing the image.
											tempSok.send(imagePacket);
											std::cout << "IMAGE SENT" << std::endl;
										}
									}
									checkWords = true; //start checking words once
									break;
								case TIM:
									//acaba el tiempo para los players y empieza un turno nuevo
									checkWords = false; //stop checking words
									for (std::list<sf::TcpSocket*>::iterator it2 = clients.begin(); it2 != clients.end(); ++it2) {
										sf::TcpSocket& tempSok = **it2;
										newPacket << commands::TIM;
										tempSok.send(newPacket);
									}
									startNewTurn = true;
									
									break;
								case DIS:
									/*
									quita al señor de players, clients y avisa al resto supongo lel
									*/
									break;

								}
							}
						}
						else if (status == sf::Socket::Disconnected) {

							DetectPlayer(client, players); //identifica al player
							if (globalPlayerPtr->turn == curTurn % playerNumber) startNewTurn = true;  //si es el que pinta empieza nuevo turno
							if (startNewTurn) std::cout << "el k dibujaba se ha desconectado" << std::endl;

							for (std::list<sf::TcpSocket*>::iterator it2 = clients.begin(); it2 != clients.end(); ++it2) {
								sf::TcpSocket& tempSok = **it2;
								sf::Packet newPacket;
								newPacket << commands::DIS << globalPlayerPtr->name << globalPlayerPtr->name;
								tempSok.send(newPacket);
							}

							selector.remove(client);
							itTemp.push_back(it); //guardamos iterador del socket desconectado

							bool found = false;
							int i = 0;
							while (!found && i < players.size()){
								if (players[i]->socket->getRemotePort() == client.getRemotePort()) {

									scoreboard.DeletePlayer(*globalPlayerPtr);
									players.erase(players.begin() + i);
									if (players.size() == 1) {
										sf::Packet packet;
										packet << commands::END << "";
										players[0]->socket->send(packet);
										running = false;
									}
									found = true;
								}
								i++;
							}
							std::cout << "Elimino el socket que se ha desconectado\n";
						}
						else {
							std::cout << "Error al recibir de " << client.getRemotePort() << std::endl;
						}
						//simular turno nuevo (hacer que el juego se acabe al llegar a max turns)
						if (startNewTurn) {
							startNewTurn = false;
							for (int i = 0; i < players.size(); i++) {
								players[i]->answered = false;
							}
							bool nextTurnPossible = false;
							while (!nextTurnPossible) {
								curTurn++;
								DetectPlayer(curTurn % playerNumber, players);
								if (globalPlayerPtr->turn == curTurn % playerNumber) { //comprueba que corresponda el turno con el jugador, por si esta desconectado
									nextTurnPossible = true;
								}
							}
							if (curTurn < maxTurns) {
								std::cout << "Turn: " << curTurn << std::endl;
								std::string word = PickWord(); //pick a word
								globalCurWord = word;
								for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it) {
									sf::TcpSocket& tempSok = **it;
									std::string playerStr = globalPlayerPtr->name;
									sf::Packet packet;
									if (globalPlayerPtr->socket->getRemotePort() == tempSok.getRemotePort()) {
										packet << commands::WRD << word;
										tempSok.send(packet);
									}
									else {
										int sizeWord = word.size();
										packet << commands::WNU << playerStr << sizeWord;
										tempSok.send(packet);
									}
								}
							}
							else {
								for (std::list<sf::TcpSocket*>::iterator it2 = clients.begin(); it2 != clients.end(); ++it2) {
									sf::TcpSocket& tempSok = **it2;
									sf::Packet newPacket;
									newPacket << commands::END << scoreboard.Winner();
									tempSok.send(newPacket);
									running = false;
								}
							}
						}
					}
				}
				//iterador que itera la lista de iteradores que iteran la lista de sockets
				for (std::list<std::list<sf::TcpSocket*>::iterator>::iterator it = itTemp.begin(); it != itTemp.end(); ++it) {
					clients.erase(*it); 
				}
				//std::list<std::list<sf::TcpSocket*>::iterator>::swap(itTemp);
			}
		}
		/*if (gameStarted && curTurn < maxTurns) {
			//simular turnos
			std::cout << "Turn: " << curTurn << std::endl;
			DetectPlayer(curTurn % playerNumber, players);
			std::string wordStr = "patata"; //pick a word
			for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it) {
				sf::TcpSocket& tempSok = **it;
				std::string playerStr = globalPlayerPtr->name;
				sf::Packet packet;
				if (globalPlayerPtr->socket->getRemotePort() == tempSok.getRemotePort()) {
					packet << commands::WRD << wordStr;
					tempSok.send(packet);
				}
				else {
					int sizeWord = wordStr.size();
					packet << commands::WNU << playerStr << sizeWord;
					tempSok.send(packet);
				}
			}
			curTurn++;
			system("pause");
		}
		*/
	}
}

void main()
{
	std::cout << "Seras servidor (s)? ... ";
	char c;
	std::cin >> c;

	if (c == 's') {
		ControlServidor();
	}
	else {
		delete globalPlayerPtr;
		exit(0);
	}
	delete globalPlayerPtr;

}