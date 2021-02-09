#include <LevelPack/Enemy.h>

EditorEnemy::EditorEnemy() {
}

EditorEnemy::EditorEnemy(int id) {
	this->id = id;
}

EditorEnemy::EditorEnemy(std::shared_ptr<const EditorEnemy> copy) {
	load(copy->format());
}

EditorEnemy::EditorEnemy(const EditorEnemy* copy) {
	load(copy->format());
}

std::shared_ptr<LevelPackObject> EditorEnemy::clone() const {
	return std::static_pointer_cast<LevelPackObject>(std::make_shared<EditorEnemy>(this));
}

std::string EditorEnemy::format() const {
	std::string res = tos(id) + formatString(name) + formatString(hitboxRadius) + formatString(health) + formatString(despawnTime) + tos(phaseIDs.size());
	for (auto t : phaseIDs) {
		res += formatTMObject(*std::get<0>(t)) + tos(std::get<1>(t)) + formatTMObject(std::get<2>(t)) + formatTMObject(std::get<3>(t));
	}
	res += tos(deathActions.size());
	for (auto action : deathActions) {
		res += formatTMObject(*action);
	}
	res += formatBool(isBoss) + formatTMObject(hurtSound) + formatTMObject(deathSound) + formatTMObject(symbolTable);
	return res;
}

void EditorEnemy::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	id = std::stoi(items.at(0));
	name = items.at(1);
	hitboxRadius = items.at(2);
	health = items.at(3);
	despawnTime = items.at(4);

	phaseIDs.clear();
	enemyPhaseCount.clear();
	int i = 6;
	for (int a = 0; a < std::stoi(items.at(5)); a++) {
		EntityAnimatableSet animatableSet;
		animatableSet.load(items.at(i + 2));
		int phaseID = std::stoi(items.at(i + 1));
		ExprSymbolTable definer;
		definer.load(items.at(i + 3));
		phaseIDs.push_back(std::make_tuple(EnemyPhaseStartConditionFactory::create(items.at(i)), phaseID, animatableSet, definer, exprtk::symbol_table<float>()));

		if (enemyPhaseCount.find(phaseID) == enemyPhaseCount.end()) {
			enemyPhaseCount[phaseID] = 1;
		} else {
			enemyPhaseCount[phaseID]++;
		}
		i += 4;
	}
	int next = i;
	deathActions.clear();
	for (i = next + 1; i < std::stoi(items.at(next)) + next + 1; i++) {
		deathActions.push_back(DeathActionFactory::create(items.at(i)));
	}
	isBoss = unformatBool(items.at(i++));
	hurtSound.load(items.at(i++));
	deathSound.load(items.at(i++));
	symbolTable.load(items.at(i++));
}

nlohmann::json EditorEnemy::toJson() {
	nlohmann::json j = {
		{"id", id},
		{"name", name},
		{"valueSymbolTable", symbolTable.toJson()},
		{"hitboxRadius", hitboxRadius},
		{"health", health},
		{"despawnTime", despawnTime},
		{"isBoss", isBoss},
		{"hurtSound", hurtSound.toJson()},
		{"deathSound", deathSound.toJson()}
	};

	nlohmann::json phaseIDsJson;
	for (auto t : phaseIDs) {
		phaseIDsJson.push_back(nlohmann::json{ {"phaseStartCondition", std::get<0>(t)->toJson()},
			{"phaseID", std::get<1>(t)}, {"animatableSet", std::get<2>(t).toJson()}, {"exprSymbolTable", std::get<3>(t).toJson()} });
	}
	j["phaseIDs"] = phaseIDsJson;

	nlohmann::json deathActionsJson;
	for (auto action : deathActions) {
		phaseIDsJson.push_back(action->toJson());
	}
	j["deathActions"] = deathActionsJson;

	return j;
}

void EditorEnemy::load(const nlohmann::json& j) {
	j.at("id").get_to(id);
	j.at("name").get_to(name);

	if (j.contains("valueSymbolTable")) {
		symbolTable.load(j.at("valueSymbolTable"));
	} else {
		symbolTable = ValueSymbolTable();
	}

	j.at("hitboxRadius").get_to(hitboxRadius);
	j.at("health").get_to(health);
	j.at("despawnTime").get_to(despawnTime);
	j.at("isBoss").get_to(isBoss);

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

	phaseIDs.clear();
	enemyPhaseCount.clear();
	if (j.contains("phaseIDs")) {
		for (const nlohmann::json& item : j.at("phaseIDs")) {
			std::shared_ptr<EnemyPhaseStartCondition> phaseStartCondition;
			int phaseID;
			EntityAnimatableSet animatableSet;
			ExprSymbolTable exprSymbolTable;

			phaseStartCondition = EnemyPhaseStartConditionFactory::create(item.at("phaseStartCondition"));
			item.at("phaseID").get_to(phaseID);
			if (item.contains("animatableSet")) {
				animatableSet.load(item.at("animatableSet"));
			}
			if (item.contains("exprSymbolTable")) {
				exprSymbolTable.load(item.at("exprSymbolTable"));
			}

			if (enemyPhaseCount.find(phaseID) == enemyPhaseCount.end()) {
				enemyPhaseCount[phaseID] = 1;
			} else {
				enemyPhaseCount[phaseID]++;
			}

			phaseIDs.push_back(std::make_tuple(phaseStartCondition, phaseID, animatableSet, exprSymbolTable, exprtk::symbol_table<float>()));
		}
	}

	deathActions.clear();
	if (j.contains("deathActions")) {
		for (nlohmann::json deathActionJson : j.at("deathActions")) {
			deathActions.push_back(DeathActionFactory::create(deathActionJson));
		}
	}
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EditorEnemy::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(hitboxRadius, hitbox radius)
	LEGAL_CHECK_EXPRESSION(health, health)
	LEGAL_CHECK_EXPRESSION(despawnTime, despawn time)

	if (phaseIDs.size() == 0) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("List of phases is empty.");
	} else {
		auto startCondition = std::get<0>(phaseIDs[0]);
		if (!std::dynamic_pointer_cast<TimeBasedEnemyPhaseStartCondition>(startCondition)) {
			status = std::max(status, LEGAL_STATUS::ILLEGAL);
			messages.push_back("The first phase start condition must be time-based with t=0");
		} else if (std::dynamic_pointer_cast<TimeBasedEnemyPhaseStartCondition>(startCondition)->getTime() != 0) {
			status = std::max(status, LEGAL_STATUS::ILLEGAL);
			messages.push_back("The first phase start condition must be time-based with t=0");
		}
	}
	int i = 0;
	for (auto t : phaseIDs) {
		// TODO: legal check EntityAnimatableSet

		auto startConditionLegal = std::get<0>(t)->legal(levelPack, spriteLoader, symbolTables);
		if (startConditionLegal.first != LEGAL_STATUS::LEGAL) {
			status = std::max(status, startConditionLegal.first);
			tabEveryLine(startConditionLegal.second);
			messages.push_back("Phase index " + std::to_string(i) + " start condition:");
			messages.insert(messages.end(), startConditionLegal.second.begin(), startConditionLegal.second.end());
		}

		if (!levelPack.hasEnemyPhase(std::get<1>(t))) {
			status = std::max(status, LEGAL_STATUS::ILLEGAL);
			messages.push_back("Phase index " + std::to_string(i) + " uses a non-existent enemy phase ID " + std::to_string(std::get<1>(t)));
		}

		if (std::get<2>(t).getAttackAnimatable().isSprite()) {
			status = std::max(status, LEGAL_STATUS::ILLEGAL);
			messages.push_back("Phase index " + std::to_string(i) + " cannot have a sprite as an attack animation.");
		}

		i++;
	}

	i = 0;
	for (auto deathAction : deathActions) {
		auto deathActionLegal = deathAction->legal(levelPack, spriteLoader, symbolTables);
		if (deathActionLegal.first != LEGAL_STATUS::LEGAL) {
			status = std::max(status, deathActionLegal.first);
			tabEveryLine(deathActionLegal.second);
			messages.push_back("Death action index " + std::to_string(i) + ":");
			messages.insert(messages.end(), deathActionLegal.second.begin(), deathActionLegal.second.end());
		}
	}

	// TODO: legal check hurtSound, deathSound

	return std::make_pair(status, messages);
}

void EditorEnemy::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(hitboxRadius)
	COMPILE_EXPRESSION_FOR_INT(health)
	COMPILE_EXPRESSION_FOR_FLOAT(despawnTime)

	for (auto t : phaseIDs) {
		std::get<0>(t)->compileExpressions(symbolTables);
		std::get<4>(t) = std::get<3>(t).toLowerLevelSymbolTable(expr);
	}
	for (auto deathAction : deathActions) {
		deathAction->compileExpressions(symbolTables);
	}
}

void EditorEnemy::setHitboxRadius(std::string hitboxRadius) {
	this->hitboxRadius = hitboxRadius;
}

void EditorEnemy::setHealth(std::string health) {
	this->health = health;
}

void EditorEnemy::setDespawnTime(std::string despawnTime) {
	this->despawnTime = despawnTime;
}

void EditorEnemy::setIsBoss(bool isBoss) {
	this->isBoss = isBoss;
}

std::vector<EntityAnimatableSet> EditorEnemy::getAnimatableSets() const {
	std::vector<EntityAnimatableSet> sets;
	for (auto t : phaseIDs) {
		sets.push_back(std::get<2>(t));
	}
	return sets;
}

std::tuple<std::shared_ptr<EnemyPhaseStartCondition>, int, EntityAnimatableSet, ExprSymbolTable, exprtk::symbol_table<float>> EditorEnemy::getPhaseData(int index) const {
	return phaseIDs[index];
}

int EditorEnemy::getPhasesCount() const {
	return phaseIDs.size();
}

float EditorEnemy::getHitboxRadius() const {
	return hitboxRadiusExprCompiledValue;
}

int EditorEnemy::getHealth() const {
	return healthExprCompiledValue;
}

float EditorEnemy::getDespawnTime() const {
	return despawnTimeExprCompiledValue;
}

const std::vector<std::shared_ptr<DeathAction>> EditorEnemy::getDeathActions() const {
	return deathActions;
}

bool EditorEnemy::getIsBoss() const {
	return isBoss;
}

SoundSettings EditorEnemy::getHurtSound() {
	return hurtSound;
}

SoundSettings EditorEnemy::getDeathSound() {
	return deathSound;
}

const std::map<int, int>* EditorEnemy::getEnemyPhaseIDsCount() const {
	return &enemyPhaseCount;
}

bool EditorEnemy::usesEnemyPhase(int enemyPhaseID) const {
	return enemyPhaseCount.find(enemyPhaseID) != enemyPhaseCount.end() && enemyPhaseCount.at(enemyPhaseID) > 0;
}

void EditorEnemy::setHurtSound(SoundSettings hurtSound) {
	this->hurtSound = hurtSound;
}

void EditorEnemy::setDeathSound(SoundSettings deathSound) {
	this->deathSound = deathSound;
}

void EditorEnemy::addDeathAction(std::shared_ptr<DeathAction> action) {
	deathActions.push_back(action);
}

void EditorEnemy::removeDeathAction(int index) {
	deathActions.erase(deathActions.begin() + index);
}

void EditorEnemy::addPhaseID(int index, std::shared_ptr<EnemyPhaseStartCondition> startCondition, int phaseID, EntityAnimatableSet animatableSet, ExprSymbolTable phaseSymbolsDefiner) {
	phaseIDs.insert(phaseIDs.begin() + index, std::make_tuple(startCondition, phaseID, animatableSet, phaseSymbolsDefiner, exprtk::symbol_table<float>()));

	if (enemyPhaseCount.find(phaseID) == enemyPhaseCount.end()) {
		enemyPhaseCount[phaseID] = 1;
	} else {
		enemyPhaseCount[phaseID]++;
	}
}

void EditorEnemy::removePhaseID(int index) {
	int phaseID = std::get<1>(phaseIDs[index]);
	phaseIDs.erase(phaseIDs.begin() + index);

	if (enemyPhaseCount.find(phaseID) == enemyPhaseCount.end()) {
		enemyPhaseCount[phaseID] = 1;
	} else {
		enemyPhaseCount[phaseID]++;
	}
}
