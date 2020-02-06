#include "Player.h"

std::string EditorPlayer::format() const {
	std::string ret = tos(initialHealth) + tos(maxHealth) + tos(speed) + tos(focusedSpeed) + tos(hitboxRadius)
		+ tos(hitboxPosX) + tos(hitboxPosY) + tos(invulnerabilityTime) + tos(powerTiers.size());
	for (PlayerPowerTier tier : powerTiers) {
		ret += formatTMObject(tier);
	}
	ret += formatTMObject(hurtSound) + formatTMObject(deathSound) + formatBool(smoothPlayerHPBar) + tos(playerHPBarColor.r) + tos(playerHPBarColor.g)
		+ tos(playerHPBarColor.b) + tos(playerHPBarColor.a) + formatTMObject(discretePlayerHPSprite) + tos(initialBombs) + tos(maxBombs)
		+ formatTMObject(bombSprite) + formatTMObject(bombReadySound);
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
	for (i = 9; i < std::stoi(items[8]) + 9; i++) {
		PlayerPowerTier tier;
		tier.load(items[i]);
		powerTiers.push_back(tier);
	}
	hurtSound.load(items[i++]);
	deathSound.load(items[i++]);
	smoothPlayerHPBar = unformatBool(items[i++]);
	playerHPBarColor = sf::Color(std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]));
	discretePlayerHPSprite.load(items[i++]);
	initialBombs = std::stoi(items[i++]);
	maxBombs = std::stoi(items[i++]);
	bombSprite.load(items[i++]);
	bombReadySound.load(items[i++]);
}

bool EditorPlayer::legal(SpriteLoader & spriteLoader, std::string & message) {
	//TODO
	return true;
}

std::string PlayerPowerTier::format() const {
	return formatTMObject(animatableSet) + tos(attackPatternID) + tos(attackPatternLoopDelay) + tos(focusedAttackPatternID)
		+ tos(focusedAttackPatternLoopDelay) + tos(bombAttackPatternID) + tos(bombCooldown);

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
