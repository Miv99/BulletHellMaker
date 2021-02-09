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
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	initialHealth = items.at(0);
	maxHealth = items.at(1);
	speed = items.at(2);
	focusedSpeed = items.at(3);
	hitboxRadius = items.at(4);
	hitboxPosX = items.at(5);
	hitboxPosY = items.at(6);
	invulnerabilityTime = items.at(7);

	powerTiers.clear();
	attackPatternIDCount.clear();
	int i;
	for (i = 9; i < std::stoi(items.at(8)) + 9; i++) {
		std::shared_ptr<PlayerPowerTier> tier = std::make_shared<PlayerPowerTier>();
		tier->load(items.at(i));
		powerTiers.push_back(tier);

		// Update attackPatternIDCount
		int attackPatternID = tier->getAttackPatternID();
		if (attackPatternIDCount.find(attackPatternID) == attackPatternIDCount.end()) {
			attackPatternIDCount[attackPatternID] = 1;
		} else {
			attackPatternIDCount[attackPatternID]++;
		}
	}
	hurtSound.load(items.at(i++));
	deathSound.load(items.at(i++));
	smoothPlayerHPBar = unformatBool(items.at(i++));
	playerHPBarColor = sf::Color(std::stof(items.at(i++)), std::stof(items.at(i++)), std::stof(items.at(i++)), std::stof(items.at(i++)));
	discretePlayerHPGuiElementFileName = items.at(i++);
	initialBombs = items.at(i++);
	maxBombs = items.at(i++);
	bombInvincibilityTime = items.at(i++);
	bombGuiElementFileName = items.at(i++);
	bombReadySound.load(items.at(i++));
	symbolTable.load(items.at(i++));
}

nlohmann::json EditorPlayer::toJson() {
	nlohmann::json j = {
		{"initialHealth", initialHealth},
		{"maxHealth", maxHealth},
		{"speed", speed},
		{"focusedSpeed", focusedSpeed},
		{"hitboxRadius", hitboxRadius},
		{"hitboxPosX", hitboxPosX},
		{"hitboxPosY", hitboxPosY},
		{"invulnerabilityTime", invulnerabilityTime},
		{"hurtSound", hurtSound.toJson()},
		{"deathSound", deathSound.toJson()},
		{"smoothPlayerHPBar", smoothPlayerHPBar},
		{"playerHPBarColor", nlohmann::json{ {"r", playerHPBarColor.r}, {"g", playerHPBarColor.g},
			{"b", playerHPBarColor.b}, {"a", playerHPBarColor.a} }},
		{"discretePlayerHPGuiElementFileName", discretePlayerHPGuiElementFileName},
		{"initialBombs", initialBombs},
		{"maxBombs", maxBombs},
		{"bombInvincibilityTime", bombInvincibilityTime},
		{"bombGuiElementFileName", bombGuiElementFileName},
		{"bombReadySound", bombReadySound.toJson()},
		{"valueSymbolTable", symbolTable.toJson()}
	};

	nlohmann::json powerTiersJson;
	for (std::shared_ptr<PlayerPowerTier> tier : powerTiers) {
		powerTiersJson.push_back(tier->toJson());
	}
	j["powerTiers"] = powerTiersJson;

	return j;
}

void EditorPlayer::load(const nlohmann::json& j) {
	j.at("initialHealth").get_to(initialHealth);
	j.at("maxHealth").get_to(maxHealth);
	j.at("speed").get_to(speed);
	j.at("focusedSpeed").get_to(focusedSpeed);
	j.at("hitboxRadius").get_to(hitboxRadius);
	j.at("hitboxPosX").get_to(hitboxPosX);
	j.at("hitboxPosY").get_to(hitboxPosY);
	j.at("invulnerabilityTime").get_to(invulnerabilityTime);
	
	if (j.contains("hurtSound")) {
		hurtSound.load(j.at("hurtSound"));
	} else {
		hurtSound = SoundSettings();
	}

	if (j.contains("deathSound")) {
		deathSound.load(j.at("deathSound"));
	} else {
		deathSound = SoundSettings();
	}

	j.at("smoothPlayerHPBar").get_to(smoothPlayerHPBar);
	j.at("playerHPBarColor").at("r").get_to(playerHPBarColor.r);
	j.at("playerHPBarColor").at("g").get_to(playerHPBarColor.g);
	j.at("playerHPBarColor").at("b").get_to(playerHPBarColor.b);
	j.at("playerHPBarColor").at("a").get_to(playerHPBarColor.a);
	j.at("discretePlayerHPGuiElementFileName").get_to(discretePlayerHPGuiElementFileName);
	j.at("initialBombs").get_to(initialBombs);
	j.at("maxBombs").get_to(maxBombs);
	j.at("bombInvincibilityTime").get_to(bombInvincibilityTime);
	j.at("bombGuiElementFileName").get_to(bombGuiElementFileName);

	if (j.contains("bombReadySound")) {
		bombReadySound.load(j.at("bombReadySound"));
	} else {
		bombReadySound = SoundSettings();
	}

	if (j.contains("valueSymbolTable")) {
		symbolTable.load(j.at("valueSymbolTable"));
	} else {
		symbolTable = ValueSymbolTable();
	}

	powerTiers.clear();
	if (j.contains("powerTiers")) {
		for (const nlohmann::json& item : j.at("powerTiers")) {
			std::shared_ptr<PlayerPowerTier> tier = std::make_shared<PlayerPowerTier>();
			tier->load(item);

			powerTiers.push_back(tier);
		}
	}
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
	return attackPatternIDCount.find(attackPatternID) != attackPatternIDCount.end() && attackPatternIDCount.at(attackPatternID) > 0;
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
	return formatTMObject(animatableSet) + tos(attackPatternID) + formatTMObject(attackPatternSymbolsDefiner) + formatString(attackPatternLoopDelay) 
		+ tos(focusedAttackPatternID) + formatString(focusedAttackPatternLoopDelay) + tos(bombAttackPatternID) + formatString(bombCooldown) 
		+ formatTMObject(symbolTable) + formatString(powerToNextTier);
}

void PlayerPowerTier::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	animatableSet.load(items.at(0));
	attackPatternID = std::stoi(items.at(1));
	attackPatternSymbolsDefiner.load(items.at(2));
	attackPatternLoopDelay = items.at(3);
	focusedAttackPatternID = std::stoi(items.at(4));
	focusedAttackPatternLoopDelay = items.at(5);
	bombAttackPatternID = std::stoi(items.at(6));
	bombCooldown = items.at(7);
	symbolTable.load(items.at(8));
	powerToNextTier = items.at(9);
}

nlohmann::json PlayerPowerTier::toJson() {
	return {
		{"animatableSet", animatableSet.toJson()},
		{"attackPatternID", attackPatternID},
		{"attackPatternSymbolsDefiner", attackPatternSymbolsDefiner.toJson()},
		{"attackPatternLoopDelay", attackPatternLoopDelay},
		{"focusedAttackPatternID", focusedAttackPatternID},
		{"focusedAttackPatternLoopDelay", focusedAttackPatternLoopDelay},
		{"bombAttackPatternID", bombAttackPatternID},
		{"bombCooldown", bombCooldown},
		{"powerToNextTier", powerToNextTier},
		{"valueSymbolTable", symbolTable.toJson()}
	};
}

void PlayerPowerTier::load(const nlohmann::json& j) {
	if (j.contains("animatableSet")) {
		animatableSet.load(j.at("animatableSet"));
	} else {
		animatableSet = EntityAnimatableSet();
	}

	j.at("attackPatternID").get_to(attackPatternID);

	if (j.contains("attackPatternSymbolsDefiner")) {
		attackPatternSymbolsDefiner.load(j.at("attackPatternSymbolsDefiner"));
	} else {
		attackPatternSymbolsDefiner = ExprSymbolTable();
	}

	j.at("attackPatternLoopDelay").get_to(attackPatternLoopDelay);
	j.at("focusedAttackPatternID").get_to(focusedAttackPatternID);
	j.at("focusedAttackPatternLoopDelay").get_to(focusedAttackPatternLoopDelay);
	j.at("bombAttackPatternID").get_to(bombAttackPatternID);
	j.at("bombCooldown").get_to(bombCooldown);
	j.at("powerToNextTier").get_to(powerToNextTier);

	if (j.contains("valueSymbolTable")) {
		symbolTable.load(j.at("valueSymbolTable"));
	} else {
		symbolTable = ValueSymbolTable();
	}
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
