#include "Player.h"

std::string EditorPlayer::format() {
	std::string ret = tos(initialHealth) + tm_delim + tos(maxHealth) + tm_delim + tos(speed) + tm_delim + tos(focusedSpeed) + tm_delim + tos(hitboxRadius) + tm_delim + tos(hitboxPosX) + tm_delim + tos(hitboxPosY) + tm_delim + tos(invulnerabilityTime);
	for (PlayerPowerTier tier : powerTiers) {
		ret += tm_delim + "(" + tier.format() + ")";
	}
	ret += tm_delim + "(" + hurtSound.format() + ")";
	ret += tm_delim + "(" + deathSound.format() + ")";
	if (smoothPlayerHPBar) {
		ret += "1" + tm_delim;
	} else {
		ret += "0" + tm_delim;
	}
	ret += tos(playerHPBarColor.r) + tm_delim + tos(playerHPBarColor.g) + tm_delim + tos(playerHPBarColor.b) + tm_delim + tos(playerHPBarColor.a) + tm_delim;
	ret += "(" + discretePlayerHPSprite.format() + ")" + tm_delim;
	ret += tos(initialBombs) + tm_delim + tos(maxBombs) + tm_delim;
	ret += "(" + bombSprite.format() + ")" + tm_delim;
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
	return "(" + animatableSet.format() + ")" + tm_delim + tos(attackPatternID) + tm_delim + tos(attackPatternLoopDelay) + tm_delim + tos(focusedAttackPatternID) + tm_delim + tos(focusedAttackPatternLoopDelay) + tm_delim + tos(bombAttackPatternID) + tm_delim + tos(bombCooldown);

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
