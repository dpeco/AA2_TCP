#pragma once
#include "SFML\Graphics.hpp"
//#include "Vector.h"

class Circle {
public:
	Circle(int r, sf::Color _color, sf::Vector2i _pos);
	~Circle();

	sf::Vector2i GetPosition();
	void draw(sf::RenderWindow* window);

private:
	int radius;
	sf::CircleShape shape;
	
};

Circle::Circle(int r, sf::Color _color, sf::Vector2i _pos) {
	radius = r;
	shape = sf::CircleShape(r);
	shape.setOrigin(r, r);
	//shape.setFillColor(_color);
	shape.setPosition(_pos.x, _pos.y);
}

Circle::~Circle() {
}

sf::Vector2i Circle::GetPosition() {
	return sf::Vector2i(shape.getPosition().x, shape.getPosition().y);
}

void Circle::draw(sf::RenderWindow* window) {
	window->draw(this->shape);
}

/*
class Line {
public:
	Line(sf::Vector2i _p1, sf::Vector2i _p2, float r);
	~Line();

	void draw(sf::RenderWindow* window);

private:
	sf::RectangleShape line;
	myVector p1, p2;
	float angle;
};

Line::Line(sf::Vector2i _p1, sf::Vector2i _p2, float r) {
	//line = sf::RectangleShape(_p1, sf::Vector2f(_p2 - _p1))
	p1 = myVector(_p1);
	p2 = myVector(_p2);
	myVector p3 = myVector(p1, p2);
	myVector p4 = myVector(p3.Length(), 0.0f);
	angle = myVector::AngleBetween(p3, p4);
	line = sf::RectangleShape();
	line.setSize(sf::Vector2f(p3.x, p3.y));
	line.setOrigin(0, line.getSize().y/2);
	line.setPosition(sf::Vector2f(p1.x, p1.y));
	
	
	//line.setFillColor(sf::Color::White);
	line.setRotation(angle);
	//line.setOutlineThickness(r);
	
}

Line::~Line()
{
}

void Line::draw(sf::RenderWindow* window) {
	window->draw(this->line);
}
*/