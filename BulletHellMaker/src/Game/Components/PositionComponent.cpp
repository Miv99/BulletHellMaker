#include <Game/Components/PositionComponent.h>

PositionComponent::PositionComponent(float x, float y)
	: x(x), y(y) {
}

float PositionComponent::getX() const {
	return x;
}

float PositionComponent::getY() const {
	return y;
}

void PositionComponent::setX(float x) {
	this->x = x;
}

void PositionComponent::setY(float y) {
	this->y = y;
}

void PositionComponent::setPosition(sf::Vector2f position) {
	x = position.x;
	y = position.y;
}