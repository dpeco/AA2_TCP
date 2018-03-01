#include <SFML\Network.hpp>
#include <iostream>
#include <list>

enum commands { NOM, DEN, CON, INF, MSG, IMG, WRD, GUD, BAD, WNU, WIN, DIS, END, RNK, RDY};

struct Player {
	std::string name = "tempname";
	bool ready = false;
	int score = 0;
	sf::TcpSocket* socket; //o algo del estilo para poder saber su port e identificarlo en la lista de clientes
};

Player* DetectPlayer(sf::TcpSocket& client, std::vector<Player*> players) { //encuentra player segun socket, dado que hara falta en varias ocasiones creo que sera util
	//encontrar player comparando remoteport
	bool found = false;
	int i = 0;
	Player* player = new Player;
	while (!found) {
		if (players[i]->socket->getRemotePort() == client.getRemotePort()) {
			player = players[i];
			found = true;
		}
		i++;
	}
	return player;
}

void ControlServidor()
{
	bool running = true;
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
					//avisamos a todos que se ha conectado un nuevo cliente
					for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it) {
						sf::TcpSocket& tempSok = **it;
						std::string newClient = "Se ha conectado un nuevo cliente";
						sf::Packet packet;
						packet << newClient;
						tempSok.send(packet);
					}

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
					
					std::string newClient = "Te has conectado al chat"; sf::Packet packet;
					packet << newClient;
					client->send(packet);

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

							//inicializa cosas antes del switch
							std::string strRec;
							Player* player;
							bool used = false;

							player = DetectPlayer(client, players); //identifica al player
							int command;
							if (packet >> command) {
								switch (command) {
								case commands::MSG:
									packet >> strRec;
									std::cout << "He recibido " << strRec << " del puerto " << client.getRemotePort() << std::endl;

									//Reenviar mensaje a todos los clientes:
									for (std::list<sf::TcpSocket*>::iterator it2 = clients.begin(); it2 != clients.end(); ++it2) {
										sf::TcpSocket& tempSok = **it2;
										sf::Packet packet;
										
										packet << commands::MSG << player->name << strRec;
										tempSok.send(packet);
									}
									break;
								case NOM:
									//compara con el resto de players si ya está usado o no
									packet >> strRec;
									for (int i = 0; i < players.size(); i++) {
										if (strcmp(players[0]->name.c_str(), strRec.c_str()) == 0) {
											used = true;
										}
									}
									//si no está usado --> send CON y añade, si lo está --> send DEN
									if (!used) {
										player->name = strRec;
										packet << commands::CON;
										client.send(packet);
									}
									else {
										packet << commands::DEN;
										client.send(packet);
									}
									break;
								case RDY:
									/*
									identifica el jugador y lo setea a ready
									*/
									break;
								case IMG:
									/*
									recibe la imagen dibujada por el cliente que ha dibujado

									quizá podria aprovechar y reenviar la imagen al resto, dependiendo de al final como va le code xd
									*/
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
							selector.remove(client);
							itTemp.push_back(it); //guardamos iterador del socket desconectado
							std::cout << "Elimino el socket que se ha desconectado\n";
						}
						else {
							std::cout << "Error al recibir de " << client.getRemotePort() << std::endl;
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
		exit(0);
	}


}