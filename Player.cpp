#include "Player.h"

std::string EditorPlayer::format() {
	return tos(initialHealth) + delim + tos(maxHealth) + delim + tos(speed) + delim + tos(focusedSpeed) + delim + "(" + animatableSet.format() + ")" + delim + tos(hitboxRadius) + delim + tos(hitboxPosX) + delim + tos(hitboxPosY);
}

void EditorPlayer::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	initialHealth = std::stoi(items[0]);
	maxHealth = std::stoi(items[1]);
	speed = std::stof(items[2]);
	focusedSpeed = std::stof(items[3]);
	animatableSet.load(items[4]);
	hitboxRadius = std::stof(items[5]);
	hitboxPosX = std::stof(items[6]);
	hitboxPosY = std::stof(items[7]);
}