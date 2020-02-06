#include "EnemySpawnCondition.h"
#include "Components.h"

std::string GlobalTimeBasedEnemySpawnCondition::format() const {
	return formatString("GlobalTimeBasedEnemySpawnCondition") + tos(time);
}

void GlobalTimeBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	time = std::stof(items[1]);
}

bool GlobalTimeBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.get<LevelManagerTag>().getTimeSinceStartOfLevel() >= time;
}

std::string EnemyCountBasedEnemySpawnCondition::format() const {
	return formatString("EnemyCountBasedEnemySpawnCondition") + tos(enemyCount);
}

void EnemyCountBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	enemyCount = std::stoi(items[1]);
}

bool EnemyCountBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.view<EnemyComponent>().size() - 1 <= enemyCount;
}

std::string TimeBasedEnemySpawnCondition::format() const {
	return formatString("TimeBasedEnemySpawnCondition") + tos(time);
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
