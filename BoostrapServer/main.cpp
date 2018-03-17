#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <iostream>
#include <list>

/*
struct Direccion { Port, IP  }
//Listener(50000)
//Cuando se conecta alguien guardo su puerto e IP bajo un vector de struct (Direccion)
for(int i = 0; i < numPlayerMinimo; i++) {
TcpSocket sock
sock.accept
if(vector.lenght > 0) { //Hay alguien esperando ya y le mandamos con un packet el número de jugadores esperando + IP y Puerto de cada usuario en espera encadenados (for que viaja por todo el vector)}
else { //No hay nadie más. Mandarle un mensaje con un 0 como número de jugadores }
vector.push(sock.getRemoteAdress, sock.getRemotePort) //En el cliente se mirará si el número de jugadores es > 0.
sock.disconnect()
}
listener.close()
*/

#define MIN_PLAYERS 4

struct Direction
{
	std::string ip;
	int port;
};
struct Player {
	std::string name;
	Direction dir;
};
std::vector<Direction> awaitingPlayers;
std::vector<Player> players;

enum commands { NOM, DEN, CON, INF};


void main() {

	sf::TcpSocket socket;
	sf::TcpListener listener;
	listener.listen(50000);
	int numplayers = 0;
	for (int i = 0; i < MIN_PLAYERS; i++) {
		std::cout << "bootstrap server listening: " << std::endl;
		sf::Socket::Status st = listener.accept(socket);
		if (st == sf::Socket::Status::Done) {
			//Miro cuántos jugadores hay esperando.
				//Si hay 0 => Le digo que no hay nadie: packet << 0
				//Si hay > 0 => Le paso un paquete con: packet << numPlayersWaiting << Ip << Port << Ip << Port << ...

			//Guardo puerto e IP de este cliente para los siguientes.

			//receive esperando nombre
			bool nameAccepted = false;
			Player curPlayer; //genera nuevo player
			curPlayer.dir.ip = socket.getRemoteAddress().toString();
			curPlayer.dir.port = socket.getRemotePort();
			
			while (!nameAccepted) {
				sf::Packet packet;
				std::cout << "server waiting for a name" << std::endl;
				st = socket.receive(packet);
				int command;
				if (packet >> command) {
					std::string strRec;
					sf::Packet newPacket;

					switch (command) {
					case commands::NOM: //no es necesario pero para asegurar
						//compara con el resto de players si ya está usado o no
						packet >> strRec;
						bool sameName = false;
						for (int i = 0; i < players.size(); i++) {
							if (strcmp(players[i].name.c_str(), strRec.c_str()) == 0) {
								sameName = true;
							}
						}
						//si no está usado --> send CON y añade, si lo está --> send DEN
						if (!sameName) {
							std::cout << "confirmed name" << std::endl;
							nameAccepted = true;
							newPacket << commands::CON;
							socket.send(newPacket);
							//guardamos name del player
							curPlayer.name = strRec;
						}
						else {
							std::cout << "denied name" << std::endl;
							newPacket << commands::DEN;
							socket.send(newPacket);
						}
						break;
					}
				}
			}

			//pasar info jugadores y numero total
			sf::Packet packet;
			packet << numplayers;
			socket.send(packet);

			if (numplayers > 0) {
				for (int i = 0; i < players.size(); i++) {
					sf::Packet tempP;
					tempP << commands::INF;
					tempP << players[i].name;
					tempP << players[i].dir.ip;
					tempP << players[i].dir.port;
					socket.send(tempP);
					std::cout << "envio info a " << players[i].dir.port << std::endl;
				}
			}
			//guardar jugador
			players.push_back(curPlayer);
			std::cout << "He guardado al jugador " << i << "\ total: " << players.size() <<  std::endl;
			numplayers++;
			/*
			sf::Packet pak;
			int playerWaiting = awaitingPlayers.size();
			std::cout << "Num players waiting: " << playerWaiting << std::endl;
			pak << playerWaiting;
			if (playerWaiting > 0) {
				for (int j = 0; j < playerWaiting; j++) {
					//
					pak << awaitingPlayers[j].ip << awaitingPlayers[j].port;
					std::cout << "Player #" << j << " IP: " << awaitingPlayers[j].ip << " PORT: " << awaitingPlayers[j].port << std::endl;
				}
			}
			socket.send(pak);
			Direction temp;
			temp.ip = socket.getRemoteAddress().toString();
			temp.port = socket.getRemotePort();
			awaitingPlayers.push_back(temp);
			std::cout << "Num players waiting after conecting player #" << i << ": " << awaitingPlayers.size() << std::endl;
			*/
		}
		socket.disconnect();
	}

	listener.close();
}