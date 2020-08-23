#pragma once
#include <SFML/Graphics.hpp>

class PositionComponent {
public:
	PositionComponent(float x = 0, float y = 0);

	float getX() const;
	float getY() const;

	void setX(float x);
	void setY(float y);
	void setPosition(sf::Vector2f position);

private:
	float x;
	float y;
};