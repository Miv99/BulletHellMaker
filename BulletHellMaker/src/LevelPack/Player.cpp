#include <LevelPack/Player.h>

EditorPlayer::EditorPlayer() {
}

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
		+ tos(playerHPBarColor.b) + tos(playerHPBarColor.a) + formatString(discretePlayerHPGuiElementFileName) + formatString(initialBombs) + formatString(maxBombs)
		+ formatString(bombInvincibilityTime) + formatString(bombGuiElementFileName) + formatTMObject(bombReadySound) + formatTMObject(symbolTable);
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
	discretePlayerHPGuiElementFileName = items[i++];
	initialBombs = items[i++];
	maxBombs = items[i++];
	bombInvincibilityTime = items[i++];
	bombGuiElementFileName = items[i++];
	bombReadySound.load(items[i++]);
	symbolTable.load(items[i++]);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EditorPlayer::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(initialHealth, initial health)
	LEGAL_CHECK_EXPRESSION(maxHealth, max health)
	LEGAL_CHECK_EXPRESSION(speed, speed)
	LEGAL_CHECK_EXPRESSION(focusedSpeed, focused speed)
	LEGAL_CHECK_EXPRESSION(hitboxRadius, hitbox radius)
	LEGAL_CHECK_EXPRESSION(hitboxPosX, hitbox x-position)
	LEGAL_CHECK_EXPRESSION(hitboxPosY, hitbox y-position)
	LEGAL_CHECK_EXPRESSION(invulnerabilityTime, invulnerability time)
	LEGAL_CHECK_EXPRESSION(initialBombs, initial bombs count)
	LEGAL_CHECK_EXPRESSION(maxBombs, max bombs count)
	LEGAL_CHECK_EXPRESSION(bombInvincibilityTime, bomb invincibility time)

	int i = 0;
	for (std::shared_ptr<PlayerPowerTier> tier : powerTiers) {
		auto tierLegal = tier->legal(levelPack, spriteLoader, symbolTables);
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

void EditorPlayer::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
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
		tier->compileExpressions(symbolTables);
	}
}

bool EditorPlayer::usesAttackPattern(int attackPatternID) const {
	return attackPatternIDCount.count(attackPatternID) > 0 && attackPatternIDCount.at(attackPatternID) > 0;
}

PlayerPowerTier::PlayerPowerTier() {
}

PlayerPowerTier::PlayerPowerTier(EntityAnimatableSet animatableSet, int attackPatternID, std::string attackPatternLoopDelay, int focusedAttackPatternID,
	std::string focusedAttackPatternLoopDelay, int bombAttackPatternID, std::string bombCooldown, std::string powerToNextTier) 
	: animatableSet(animatableSet), attackPatternID(attackPatternID), attackPatternLoopDelay(attackPatternLoopDelay), focusedAttackPatternID(focusedAttackPatternID),
	focusedAttackPatternLoopDelay(focusedAttackPatternLoopDelay), bombAttackPatternID(bombAttackPatternID), bombCooldown(bombCooldown), powerToNextTier(powerToNextTier) {
}

std::shared_ptr<LevelPackObject> PlayerPowerTier::clone() const {
	std::shared_ptr<PlayerPowerTier> clone = std::make_shared<PlayerPowerTier>();
	clone->load(format());
	return clone;
}

std::string PlayerPowerTier::format() const {
	return formatTMObject(animatableSet) + tos(attackPatternID) + formatString(attackPatternLoopDelay) + tos(focusedAttackPatternID)
		+ formatString(focusedAttackPatternLoopDelay) + tos(bombAttackPatternID) + formatString(bombCooldown) 
		+ formatTMObject(symbolTable) + formatString(powerToNextTier);
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
	powerToNextTier = items[8];
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> PlayerPowerTier::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(attackPatternLoopDelay, attack pattern loop delay)
	LEGAL_CHECK_EXPRESSION(focusedAttackPatternLoopDelay, focused attack pattern loop delay)
	LEGAL_CHECK_EXPRESSION(bombCooldown, bomb cooldown)
	LEGAL_CHECK_EXPRESSION(powerToNextTier, power to next tier)

	// TODO: check animatableSet, attackPatternID, focusedAttackPatternID, bombAttackPatternID
	return std::make_pair(status, messages);
}

void PlayerPowerTier::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(attackPatternLoopDelay)
	COMPILE_EXPRESSION_FOR_FLOAT(focusedAttackPatternLoopDelay)
	COMPILE_EXPRESSION_FOR_FLOAT(bombCooldown)
	COMPILE_EXPRESSION_FOR_INT(powerToNextTier)

	compiledAttackPatternSymbolsDefiner = attackPatternSymbolsDefiner.toLowerLevelSymbolTable(expr);
}
