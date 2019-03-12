#pragma once
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <entt/entt.hpp>
#include "TextMarshallable.h"
#include "EnemyPhaseAction.h"

/*
The condition for an enemy phase to start.
*/
class EnemyPhaseStartCondition : public TextMarshallable {
public:
	std::string format() = 0;
	void load(std::string formattedString) = 0;

	virtual bool satisfied(entt::DefaultRegistry& registry, uint32_t entity) = 0;
};

/*
EnemyPhaseStartCondition that depends on the time since the enemy's spawn.
*/
class TimeBasedEnemyPhaseStartCondition : public EnemyPhaseStartCondition {
public:
	inline TimeBasedEnemyPhaseStartCondition() {}
	inline TimeBasedEnemyPhaseStartCondition(float time) : time(time) {}

	std::string format() override;
	void load(std::string formattedString) override;

	bool satisfied(entt::DefaultRegistry& registry, uint32_t entity) override;

private:
	// Minimum time since the enemy's spawn for this condition to be satisfied
	float time;
};

/*
EnemyPhaseStartCondition that depends on the enemy's percent health remaining.
*/
class HPBasedEnemyPhaseStartCondition : public EnemyPhaseStartCondition {
public:
	inline HPBasedEnemyPhaseStartCondition() {}
	inline HPBasedEnemyPhaseStartCondition(float percent) : percent(percent) {}

	std::string format() override;
	void load(std::string formattedString) override;

	bool satisfied(entt::DefaultRegistry& registry, uint32_t entity) override;

private:
	// Maximum hp percent of the enemy for this condition to be satisfied
	float percent;
};

/*
EnemyPhaseStartCondition that depends on the number of other enemies alive.
The enemy that this condition belongs to does not count.
*/
class EnemyCountBasedEnemyPhaseStartCondition : public EnemyPhaseStartCondition {
public:
	inline EnemyCountBasedEnemyPhaseStartCondition() {}
	inline EnemyCountBasedEnemyPhaseStartCondition(int enemyCount) : enemyCount(enemyCount) {}

	std::string format() override;
	void load(std::string formattedString) override;

	bool satisfied(entt::DefaultRegistry& registry, uint32_t entity) override;

private:
	// Maximum number of other enemies alive for this condition to be satisfied
	int enemyCount;
};

/*
The factory for creating EnemyPhaseStartConditions.
Creates the correct concrete EnemyPhaseStartConditions using the formatted string.
*/
class EnemyPhaseStartConditionFactory {
public:
	static std::shared_ptr<EnemyPhaseStartCondition> create(std::string formattedString);
};