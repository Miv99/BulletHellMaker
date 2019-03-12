#include "EnemySpawnCondition.h"
#include "Components.h"

std::string GlobalTimeBasedEnemySpawnCondition::format() {
	std::string res = "";
	res += "GlobalTimeBasedEnemySpawnCondition" + delim;
	res += "(" + tos(time) + ")";
	return res;
}

void GlobalTimeBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	time = std::stof(items[1]);
}

bool GlobalTimeBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.get<LevelManagerTag>().getTimeSinceStartOfLevel() >= time;
}

std::string EnemyCountBasedEnemySpawnCondition::format() {
	std::string res = "";
	res += "EnemyCountBasedEnemySpawnCondition" + delim;
	res += "(" + tos(enemyCount) + ")";
	return res;
}

void EnemyCountBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	enemyCount = std::stoi(items[1]);
}

bool EnemyCountBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.view<EnemyComponent>().size() - 1 <= enemyCount;
}

std::string TimeBasedEnemySpawnCondition::format() {
	std::string res = "";
	res += "TimeBasedEnemySpawnCondition" + delim;
	res += "(" + tos(time) + ")";
	return res;
}

void TimeBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	time = std::stof(items[1]);
}

bool TimeBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.get<LevelManagerTag>().getTimeSinceLastEnemySpawn() >= time;
}

std::shared_ptr<EnemySpawnCondition> EnemySpawnConditionFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<EnemySpawnCondition> ptr;
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
