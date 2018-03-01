#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>
#include <time.h>

#define MAX_MENSAJES 25

std::vector<std::string> aMensajes;
std::mutex myMutex;
bool connected = false;
std::thread receiveThread;
sf::IpAddress ip = sf::IpAddress::getLocalAddress();
sf::TcpSocket socket;
char connectionType, mode;
char buffer[2000];
std::size_t received;
std::string text = "Connected to: ";
int ticks = 0;
std::string windowName;
sf::Socket::Status st;
sf::Color color;
std::string mensaje;


enum commands { NOM, DEN, CON, INF, MSG, IMG, WRD, GUD, BAD, WNU, WIN, DIS, END, RNK, RDY};

void addMessage(std::string s) {
	std::lock_guard<std::mutex> guard(myMutex);
	aMensajes.push_back(s);
	if (aMensajes.size() > 25) {
		aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
	}
}

void receiveFunction(sf::TcpSocket* socket, bool* _connected) {
	char receiveBuffer[2000];
	std::size_t _received;
	while (*_connected) {
		sf::Packet packet;
		sf::Socket::Status rSt = socket->receive(packet);
		if (rSt == sf::Socket::Status::Done/*_received > 0*/) {

			std::string str;
			std::string str2;

			int command;
			if (packet >> command) {
				switch (command) {
				case commands::DEN:
					//el server no guarda el nombre y se repite el proceso de introducir nombre
					break;
				case commands::CON: //temporal para solo recibir 1 string
					//el server guarda el nombre

					packet >> str;
					addMessage(str);
					break;
				case commands::RNK: 

					packet >> str;
					addMessage(str);
					break;
				case commands::INF:
					break;
				case commands::MSG:
					packet >> str >> str2;
					addMessage(str + str2);

					if (strcmp(str2.c_str(), " >exit") == 0) {
						std::cout << "EXIT" << std::endl;
						//*_connected = false;
						//std::string exitMessage = " >exit";
						//socket->send(exitMessage.c_str(), exitMessage.length() + 1);
						addMessage("OTHER USER DISONNECTED FROM CHAT");
					}
					break;
				case commands::IMG:
					//recibir la imagen para printarla en window
					break;
				case commands::WRD:
					//poner un mensaje indicando la palabra que hay que dibujar
					break;
				case commands::WNU:
					break;
				case commands::BAD:
					//poner un mensaje indicando que has escrito mal le palabra
					break;
				case commands::GUD:
					//poner un mensaje indicando que has escrito bien la palabra
					break;
				case commands::WIN:
					//poner mensaje indicando que un jugador ha acertado la palabra

					//actualizar scoreboard local, actualizando la puntuacion del jugador que ha acertado
				case commands::DIS:
					//poner un mensaje diciendo que alguien se ha desconectado (podria enviar el nombre tbn)
					break;
				case commands::END:
					//mensaje indicando el ganador de la partida, indicando su nombre
					break;

				}
			}
		}
	}
}

void blockeComunication() {

	receiveThread = std::thread(receiveFunction, &socket, &connected);

	bool done = false;
	while (!done && (st == sf::Socket::Status::Done) && connected)
	{

		sf::Vector2i screenDimensions(800, 600);

		sf::RenderWindow window;
		window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), windowName);

		sf::Font font;
		if (!font.loadFromFile("courbd.ttf"))
		{
			std::cout << "Can't load the font file" << std::endl;
		}

		std::string mensaje = "";

		sf::Text chattingText(mensaje, font, 14);

		chattingText.setFillColor(color);
		chattingText.setStyle(sf::Text::Regular);


		sf::Text text(mensaje, font, 14);
		text.setFillColor(color);
		text.setStyle(sf::Text::Regular);
		text.setPosition(0, 560);

		sf::RectangleShape separator(sf::Vector2f(800, 5));
		separator.setFillColor(sf::Color(200, 200, 200, 255));
		separator.setPosition(0, 550);

		while (window.isOpen())
		{
			sf::Event evento;
			while (window.pollEvent(evento))
			{
				std::string exitMessage;
				sf::Packet packet;
				switch (evento.type)
				{
				case sf::Event::Closed:
					//DISCONECT FROM SERVER
					done = true;
					std::cout << "CLOSE" << std::endl;
					connected = false;
					exitMessage = " >exit";
					packet << commands::MSG << exitMessage;
					socket.send(packet);
					window.close();
					break;
				case sf::Event::KeyPressed:
					if (evento.key.code == sf::Keyboard::Escape)
						window.close();
					else if (evento.key.code == sf::Keyboard::Return) //envia mensaje
					{
						sf::Packet packet;
						packet << commands::MSG << (" >" + mensaje).c_str();
						sf::Socket::Status tempSt = socket.send(packet);
						//addMessage(mensaje);
						if (strcmp(mensaje.c_str(), "exit") == 0) {
							std::cout << "EXIT" << std::endl;
							//addMessage("YOU DISCONNECTED FROM CHAT");
							connected = false;
							done = true;
							window.close();
						}
						mensaje = "";
					}
					break;
				case sf::Event::TextEntered:
					if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
						mensaje += (char)evento.text.unicode;
					else if (evento.text.unicode == 8 && mensaje.length() > 0)
						mensaje.erase(mensaje.length() - 1, mensaje.length());
					break;
				}
			}

			window.draw(separator);
			for (size_t i = 0; i < aMensajes.size(); i++)
			{
				std::string chatting = aMensajes[i];
				chattingText.setPosition(sf::Vector2f(0, 20 * i));
				chattingText.setString(chatting);
				window.draw(chattingText);
			}
			std::string mensaje_ = mensaje + "_";
			text.setString(mensaje_);
			window.draw(text);


			window.display();
			window.clear();
		}
		receiveThread.join();
	}
}

void main() {
	srand(time(NULL));
	color = sf::Color(rand() % 255 + 0, rand() % 255 + 0, rand() % 255 + 0, 255);
	st = sf::Socket::Status::Disconnected;
	bool serv;
	std::string serverMode;

	std::cout << "Enter (c) for Client: ";
	std::cin >> connectionType;

	if (connectionType == 'c')
	{
		serv = false;
		do {
			ticks++;
			st = socket.connect(ip, 50000, sf::seconds(5.f));
			if (st != sf::Socket::Status::Done) std::cout << "NO SE PUDO CONECTAR PENDEJO TRAS 5s" << std::endl;
		} while (st != sf::Socket::Status::Done && ticks < 3);

		text += "Client";
		mode = 'r';
		windowName = "Client Chat Window";

	}

	if (st == sf::Socket::Status::Done) {
		connected = true;
		blockeComunication();
	}

	socket.disconnect();
}