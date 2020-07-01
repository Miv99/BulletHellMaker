#include "Enemy.h"

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
	res += formatBool(isBoss) + formatTMObject(hurtSound) + formatTMObject(deathSound);
	return res;
}

void EditorEnemy::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	id = std::stoi(items[0]);
	name = items[1];
	hitboxRadius = items[2];
	health = items[3];
	despawnTime = items[4];

	phaseIDs.clear();
	enemyPhaseCount.clear();
	int i = 6;
	for (int a = 0; a < std::stoi(items[5]); a++) {
		EntityAnimatableSet animatableSet;
		animatableSet.load(items[i + 2]);
		int phaseID = std::stoi(items[i + 1]);
		ExprSymbolTable definer;
		definer.load(items[i + 3]);
		phaseIDs.push_back(std::make_tuple(EnemyPhaseStartConditionFactory::create(items[i]), phaseID, animatableSet, definer, exprtk::symbol_table<float>()));

		if (enemyPhaseCount.count(phaseID) == 0) {
			enemyPhaseCount[phaseID] = 1;
		} else {
			enemyPhaseCount[phaseID]++;
		}
		i += 4;
	}
	int next = i;
	deathActions.clear();
	for (i = next + 1; i < std::stoi(items[next]) + next + 1; i++) {
		deathActions.push_back(DeathActionFactory::create(items[i]));
	}
	isBoss = unformatBool(items[i++]);
	hurtSound.load(items[i++]);
	deathSound.load(items[i++]);
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
			messages.push_back("Phase index " + std::to_string(i) + " uses a non-existent enemy phase ID:");
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

bool EditorEnemy::usesEnemyPhase(int enemyPhaseID) const {
	return enemyPhaseCount.count(enemyPhaseID) > 0 && enemyPhaseCount.at(enemyPhaseID) > 0;
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

	if (enemyPhaseCount.count(phaseID) == 0) {
		enemyPhaseCount[phaseID] = 1;
	} else {
		enemyPhaseCount[phaseID]++;
	}
}

void EditorEnemy::removePhaseID(int index) {
	int phaseID = std::get<1>(phaseIDs[index]);
	phaseIDs.erase(phaseIDs.begin() + index);

	if (enemyPhaseCount.count(phaseID) == 0) {
		enemyPhaseCount[phaseID] = 1;
	} else {
		enemyPhaseCount[phaseID]++;
	}
}
