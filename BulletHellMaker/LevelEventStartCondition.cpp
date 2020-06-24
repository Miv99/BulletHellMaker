#include "LevelEventStartCondition.h"
#include "Components.h"

std::shared_ptr<LevelPackObject> GlobalTimeBasedEnemySpawnCondition::clone() const {
	auto clone = std::make_shared<GlobalTimeBasedEnemySpawnCondition>();
	clone->load(format());
	return clone;
}

std::string GlobalTimeBasedEnemySpawnCondition::format() const {
	return formatString("GlobalTimeBasedEnemySpawnCondition") + tos(time);
}

void GlobalTimeBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	time = std::stof(items[1]);
}

std::pair<bool, std::string> GlobalTimeBasedEnemySpawnCondition::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	// TODO
	return std::pair<bool, std::string>();
}

void GlobalTimeBasedEnemySpawnCondition::compileExpressions(exprtk::symbol_table<float> symbolTable) {
	// TODO
}

bool GlobalTimeBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.get<LevelManagerTag>().getTimeSinceStartOfLevel() >= time;
}

std::shared_ptr<LevelPackObject> EnemyCountBasedEnemySpawnCondition::clone() const {
	auto clone = std::make_shared<EnemyCountBasedEnemySpawnCondition>();
	clone->load(format());
	return clone;
}

std::string EnemyCountBasedEnemySpawnCondition::format() const {
	return formatString("EnemyCountBasedEnemySpawnCondition") + tos(enemyCount);
}

void EnemyCountBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	enemyCount = std::stoi(items[1]);
}

std::pair<bool, std::string> EnemyCountBasedEnemySpawnCondition::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	// TODO
	return std::pair<bool, std::string>();
}

void EnemyCountBasedEnemySpawnCondition::compileExpressions(exprtk::symbol_table<float> symbolTable) {
	// TODO
}

bool EnemyCountBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.view<EnemyComponent>().size() - 1 <= enemyCount;
}

std::shared_ptr<LevelPackObject> TimeBasedEnemySpawnCondition::clone() const {
	auto clone = std::make_shared<TimeBasedEnemySpawnCondition>();
	clone->load(format());
	return clone;
}

std::string TimeBasedEnemySpawnCondition::format() const {
	return formatString("TimeBasedEnemySpawnCondition") + tos(time);
}

void TimeBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	time = std::stof(items[1]);
}

std::pair<bool, std::string> TimeBasedEnemySpawnCondition::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	// TODO
	return std::pair<bool, std::string>();
}

void TimeBasedEnemySpawnCondition::compileExpressions(exprtk::symbol_table<float> symbolTable) {
	// TODO
}

bool TimeBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.get<LevelManagerTag>().getTimeSinceLastEnemySpawn() >= time;
}

std::shared_ptr<LevelEventStartCondition> LevelEventStartConditionFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<LevelEventStartCondition> ptr;
	if (name == "GlobalTimeBasedEnemySpawnCondition") {
		ptr = std::make_shared<GlobalTimeBasedEnemySpawnCondition>();
	} else if (name == "TimeBasedEnemySpawnCondition") {
		ptr = std::make_shared<TimeBasedEnemySpawnCondition>();
	} else if (name == "EnemyCountBasedEnemySpawnCondition") {
		ptr = std::make_shared<EnemyCountBasedEnemySpawnCondition>();
	}
	ptr->load(formattedString);
	return ptr;
}
