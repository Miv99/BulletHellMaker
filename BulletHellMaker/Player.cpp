#include "Player.h"

std::shared_ptr<LevelPackObject> EditorPlayer::clone() const {
	std::shared_ptr<EditorPlayer> clone = std::make_shared<EditorPlayer>();
	clone->load(format());
	return clone;
}

std::string EditorPlayer::format() const {
	std::string ret = formatString(initialHealth) + formatString(maxHealth) + formatString(speed) + formatString(focusedSpeed) + formatString(hitboxRadius)
		+ formatString(hitboxPosX) + formatString(hitboxPosY) + formatString(invulnerabilityTime) + tos(powerTiers.size());
	for (std::shared_ptr<PlayerPowerTier> tier : powerTiers) {
		ret += formatTMObject(*tier);
	}
	ret += formatTMObject(hurtSound) + formatTMObject(deathSound) + formatBool(smoothPlayerHPBar) + tos(playerHPBarColor.r) + tos(playerHPBarColor.g)
		+ tos(playerHPBarColor.b) + tos(playerHPBarColor.a) + formatTMObject(discretePlayerHPSprite) + formatString(initialBombs) + formatString(maxBombs)
		+ formatString(bombInvincibilityTime) + formatTMObject(bombSprite) + formatTMObject(bombReadySound) + formatTMObject(symbolTable);
	return ret;
}

void EditorPlayer::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	initialHealth = items[0];
	maxHealth = items[1];
	speed = items[2];
	focusedSpeed = items[3];
	hitboxRadius = items[4];
	hitboxPosX = items[5];
	hitboxPosY = items[6];
	invulnerabilityTime = items[7];

	powerTiers.clear();
	attackPatternIDCount.clear();
	int i;
	for (i = 9; i < std::stoi(items[8]) + 9; i++) {
		std::shared_ptr<PlayerPowerTier> tier = std::make_shared<PlayerPowerTier>();
		tier->load(items[i]);
		powerTiers.push_back(tier);

		// Update attackPatternIDCount
		int attackPatternID = tier->getAttackPatternID();
		if (attackPatternIDCount.count(attackPatternID) == 0) {
			attackPatternIDCount[attackPatternID] = 1;
		} else {
			attackPatternIDCount[attackPatternID]++;
		}
	}
	hurtSound.load(items[i++]);
	deathSound.load(items[i++]);
	smoothPlayerHPBar = unformatBool(items[i++]);
	playerHPBarColor = sf::Color(std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]));
	discretePlayerHPSprite.load(items[i++]);
	initialBombs = items[i++];
	maxBombs = items[i++];
	bombInvincibilityTime = items[i++];
	bombSprite.load(items[i++]);
	bombReadySound.load(items[i++]);
	symbolTable.load(items[i++]);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EditorPlayer::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	exprtk::parser<float> parser;
	if (!expressionStrIsValid(parser, initialHealth, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for initial health");
	}
	if (!expressionStrIsValid(parser, maxHealth, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for max health");
	}
	if (!expressionStrIsValid(parser, speed, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for speed");
	}
	if (!expressionStrIsValid(parser, focusedSpeed, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for focused speed");
	}
	if (!expressionStrIsValid(parser, hitboxRadius, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for hitbox radius");
	}
	if (!expressionStrIsValid(parser, hitboxPosX, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for hitbox x-position");
	}
	if (!expressionStrIsValid(parser, hitboxPosY, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for hitbox y-position");
	}
	if (!expressionStrIsValid(parser, invulnerabilityTime, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for invulnerability time");
	}
	if (!expressionStrIsValid(parser, initialBombs, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for initial bombs");
	}
	if (!expressionStrIsValid(parser, maxBombs, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for max bombs");
	}
	if (!expressionStrIsValid(parser, bombInvincibilityTime, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for bomb invincibility time");
	}

	int i = 0;
	for (std::shared_ptr<PlayerPowerTier> tier : powerTiers) {
		auto tierLegal = tier->legal(levelPack, spriteLoader);
		if (tierLegal.first != LEGAL_STATUS::LEGAL) {
			status = std::max(status, tierLegal.first);
			tabEveryLine(tierLegal.second);
			messages.push_back("Power tier index " + std::to_string(i) + ":");
			messages.insert(messages.end(), tierLegal.second.begin(), tierLegal.second.end());
		}

		i++;
	}

	// TODO: legal check hurtSound, deathSound, discretePlayerHPSprite (only check if smoothPlayerHPBar == false), bombSprite, bombReadySound
	return std::make_pair(status, messages);
}

void EditorPlayer::compileExpressions(exprtk::symbol_table<float> symbolTable) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(initialHealth)
	COMPILE_EXPRESSION_FOR_FLOAT(maxHealth)
	COMPILE_EXPRESSION_FOR_FLOAT(speed)
	COMPILE_EXPRESSION_FOR_FLOAT(focusedSpeed)
	COMPILE_EXPRESSION_FOR_FLOAT(hitboxRadius)
	COMPILE_EXPRESSION_FOR_FLOAT(hitboxPosX)
	COMPILE_EXPRESSION_FOR_FLOAT(hitboxPosY)
	COMPILE_EXPRESSION_FOR_FLOAT(invulnerabilityTime)
	COMPILE_EXPRESSION_FOR_INT(initialBombs)
	COMPILE_EXPRESSION_FOR_INT(maxBombs)
	COMPILE_EXPRESSION_FOR_FLOAT(bombInvincibilityTime)

	for (auto tier : powerTiers) {
		tier->compileExpressions(symbolTable);
	}
}

std::shared_ptr<LevelPackObject> PlayerPowerTier::clone() const {
	std::shared_ptr<PlayerPowerTier> clone = std::make_shared<PlayerPowerTier>();
	clone->load(format());
	return clone;
}

std::string PlayerPowerTier::format() const {
	return formatTMObject(animatableSet) + tos(attackPatternID) + formatString(attackPatternLoopDelay) + tos(focusedAttackPatternID)
		+ formatString(focusedAttackPatternLoopDelay) + tos(bombAttackPatternID) + formatString(bombCooldown) + formatTMObject(symbolTable);
}

void PlayerPowerTier::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatableSet.load(items[0]);
	attackPatternID = std::stoi(items[1]);
	attackPatternLoopDelay = items[2];
	focusedAttackPatternID = std::stoi(items[3]);
	focusedAttackPatternLoopDelay = items[4];
	bombAttackPatternID = std::stoi(items[5]);
	bombCooldown = items[6];
	symbolTable.load(items[7]);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> PlayerPowerTier::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	exprtk::parser<float> parser;
	if (!expressionStrIsValid(parser, attackPatternLoopDelay, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for attack pattern loop delay");
	}
	if (!expressionStrIsValid(parser, focusedAttackPatternLoopDelay, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for focused attack pattern loop delay");
	}
	if (!expressionStrIsValid(parser, bombCooldown, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for bomb cooldown");
	}
	// TODO: check animatableSet, attackPatternID, focusedAttackPatternID, bombAttackPatternID
	return std::make_pair(status, messages);
}

void PlayerPowerTier::compileExpressions(exprtk::symbol_table<float> symbolTable) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(attackPatternLoopDelay)
	COMPILE_EXPRESSION_FOR_FLOAT(focusedAttackPatternLoopDelay)
	COMPILE_EXPRESSION_FOR_FLOAT(bombCooldown)
}
