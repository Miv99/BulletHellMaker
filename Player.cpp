#include "Player.h"

std::string EditorPlayer::format() {
	std::string ret = tos(initialHealth) + delim + tos(maxHealth) + delim + tos(speed) + delim + tos(focusedSpeed) + delim + tos(hitboxRadius) + delim + tos(hitboxPosX) + delim + tos(hitboxPosY) + delim + tos(invulnerabilityTime);
	for (PlayerPowerTier tier : powerTiers) {
		ret += delim + "(" + tier.format() + ")";
	}
	ret += delim + "(" + hurtSound.format() + ")";
	ret += delim + "(" + deathSound.format() + ")";
	if (smoothPlayerHPBar) {
		ret += "1" + delim;
	} else {
		ret += "0" + delim;
	}
	ret += tos(playerHPBarColor.r) + delim + tos(playerHPBarColor.g) + delim + tos(playerHPBarColor.b) + delim + tos(playerHPBarColor.a) + delim;
	ret += "(" + discretePlayerHPSprite.format() + ")" + delim;
	ret += tos(initialBombs) + delim + tos(maxBombs) + delim;
	ret += "(" + bombSprite.format() + ")" + delim;
	ret += "(" + bombReadySound.format() + ")";
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
	invulnerabilityTime = std::stof(items[7]);
	int i;
	for (i = 8; i < items.size(); i++) {
		PlayerPowerTier tier;
		tier.load(items[i]);
		powerTiers.push_back(tier);
	}
	hurtSound.load(items[i++]);
	deathSound.load(items[i++]);
	if (std::stoi(items[i++]) == 1) {
		smoothPlayerHPBar = true;
	} else {
		smoothPlayerHPBar = false;
	}
	playerHPBarColor = sf::Color(std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]));
	discretePlayerHPSprite.load(items[i++]);
	initialBombs = std::stoi(items[i++]);
	maxBombs = std::stoi(items[i++]);
	bombSprite.load(items[i++]);
	bombReadySound.load(items[i++]);
}

std::string PlayerPowerTier::format() {
	return "(" + animatableSet.format() + ")" + delim + tos(attackPatternID) + delim + tos(attackPatternLoopDelay) + delim + tos(focusedAttackPatternID) + delim + tos(focusedAttackPatternLoopDelay) + delim + tos(bombAttackPatternID) + delim + tos(bombCooldown);

}

void PlayerPowerTier::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatableSet.load(items[0]);
	attackPatternID = std::stoi(items[1]);
	attackPatternLoopDelay = std::stof(items[2]);
	focusedAttackPatternID = std::stoi(items[3]);
	focusedAttackPatternLoopDelay = std::stof(items[4]);
	bombAttackPatternID = std::stoi(items[5]);
	bombCooldown = std::stof(items[6]);
}
