#include "EnemyPhaseStartCondition.h"
#include "Components.h"

std::string TimeBasedEnemyPhaseStartCondition::format() const {
	std::string res = "";
	res += "TimeBasedEnemyPhaseStartCondition" + tm_delim;
	res += "(" + tos(time) + ")";
	return res;
}

void TimeBasedEnemyPhaseStartCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	time = std::stof(items[1]);
}

bool TimeBasedEnemyPhaseStartCondition::satisfied(entt::DefaultRegistry & registry, uint32_t entity) {
	return registry.get<EnemyComponent>(entity).getTimeSinceLastPhase() >= time;
}

std::string HPBasedEnemyPhaseStartCondition::format() const {
	std::string res = "";
	res += "HPBasedEnemyPhaseStartCondition" + tm_delim;
	res += "(" + tos(ratio) + ")";
	return res;
}

void HPBasedEnemyPhaseStartCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	ratio = std::stof(items[1]);
}

bool HPBasedEnemyPhaseStartCondition::satisfied(entt::DefaultRegistry & registry, uint32_t entity) {
	auto& health = registry.get<HealthComponent>(entity);
	return health.getHealth()/health.getMaxHealth() <= ratio;
}

std::string EnemyCountBasedEnemyPhaseStartCondition::format() const {
	std::string res = "";
	res += "EnemyCountBasedEnemyPhaseStartCondition" + tm_delim;
	res += "(" + tos(enemyCount) + ")";
	return res;
}

void EnemyCountBasedEnemyPhaseStartCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	enemyCount = std::stoi(items[1]);
}

bool EnemyCountBasedEnemyPhaseStartCondition::satisfied(entt::DefaultRegistry & registry, uint32_t entity) {
	return registry.view<EnemyComponent>().size() - 1 <= enemyCount;
}

std::shared_ptr<EnemyPhaseStartCondition> EnemyPhaseStartConditionFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<EnemyPhaseStartCondition> ptr;
	if (name == "TimeBasedEnemyPhaseStartCondition") {
		ptr = std::make_shared<TimeBasedEnemyPhaseStartCondition>();
	}
	else if (name == "HPBasedEnemyPhaseStartCondition") {
		ptr = std::make_shared<HPBasedEnemyPhaseStartCondition>();
	}
	else if (name == "EnemyCountBasedEnemyPhaseStartCondition") {
		ptr = std::make_shared<EnemyCountBasedEnemyPhaseStartCondition>();
	}
	ptr->load(formattedString);
	return ptr;
}