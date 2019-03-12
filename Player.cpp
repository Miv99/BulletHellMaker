#include "Player.h"

std::string EditorPlayer::format() {
	return tos(initialHealth) + delim + tos(maxHealth) + delim + tos(speed) + delim + tos(focusedSpeed);
}

void EditorPlayer::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	initialHealth = std::stoi(items[0]);
	maxHealth = std::stoi(items[1]);
	speed = std::stof(items[2]);
	focusedSpeed = std::stof(items[3]);
}