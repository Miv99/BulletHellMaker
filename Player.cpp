#include "Player.h"

std::string EditorPlayer::format() {
	std::string ret = tos(initialHealth) + delim + tos(maxHealth) + delim + tos(speed) + delim + tos(focusedSpeed) + delim + tos(hitboxRadius) + delim + tos(hitboxPosX) + delim + tos(hitboxPosY);
	for (PlayerPowerTier tier : powerTiers) {
		ret += delim + "(" + tier.format() + ")";
	}
	return ret;
}

void EditorPlayer::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	initialHealth = std::stoi(items[0]);
	maxHealth = std::stoi(items[1]);
	speed = std::stof(items[2]);
	focusedSpeed = std::stof(items[3]);
	hitboxRadius = std::stof(items[4]);
	hitboxPosX = std::stof(items[5]);
	hitboxPosY = std::stof(items[6]);
	for (int i = 7; i < items.size(); i++) {
		PlayerPowerTier tier;
		tier.load(items[i]);
		powerTiers.push_back(tier);
	}
}

std::string PlayerPowerTier::format() {
	return "(" + animatableSet.format() + ")" + delim + tos(attackPatternID) + delim + tos(attackPatternLoopDelay) + delim + tos(focusedAttackPatternID) + delim + tos(focusedAttackPatternLoopDelay);
}

void PlayerPowerTier::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatableSet.load(items[0]);
	attackPatternID = std::stoi(items[1]);
	attackPatternLoopDelay = std::stof(items[2]);
	focusedAttackPatternID = std::stoi(items[3]);
	focusedAttackPatternLoopDelay = std::stof(items[4]);
}
