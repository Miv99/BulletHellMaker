#pragma once
#include <vector>
#include <utility>
#include <string>
#include <entt/entt.hpp>
#include "TextMarshallable.h"

class LevelEventStartCondition : public TextMarshallable {
public:
	std::string format() const = 0;
	void load(std::string formattedString) = 0;

	virtual bool satisfied(entt::DefaultRegistry& registry) = 0;
};

/*
LevelEventStartCondition that depends on the time since the start of the level.
*/
class GlobalTimeBasedEnemySpawnCondition : public LevelEventStartCondition {
public:
	inline GlobalTimeBasedEnemySpawnCondition() {}
	inline GlobalTimeBasedEnemySpawnCondition(float time) : time(time) {}

	std::string format() const override;
	void load(std::string formattedString) override;

	bool satisfied(entt::DefaultRegistry& registry) override;

private:
	// Minimum time since the start of the level for this condition to be satisfied
	float time;
};

/*
LevelEventStartCondition that depends on the time since the last enemy spawn.
*/
class TimeBasedEnemySpawnCondition : public LevelEventStartCondition {
public:
	inline TimeBasedEnemySpawnCondition() {}
	inline TimeBasedEnemySpawnCondition(float time) : time(time) {}

	std::string format() const override;
	void load(std::string formattedString) override;

	bool satisfied(entt::DefaultRegistry& registry) override;

private:
	// Minimum time since the last enemy's spawn for this condition to be satisfied
	float time;
};

/*
LevelEventStartCondition that depends on the number of enemies alive.
*/
class EnemyCountBasedEnemySpawnCondition : public LevelEventStartCondition {
public:
	inline EnemyCountBasedEnemySpawnCondition() {}
	inline EnemyCountBasedEnemySpawnCondition(int enemyCount) : enemyCount(enemyCount) {}

	std::string format() const override;
	void load(std::string formattedString) override;

	bool satisfied(entt::DefaultRegistry& registry) override;

private:
	// Maximum number of other enemies alive for this condition to be satisfied
	int enemyCount;
};

/*
The factory for creating EnemyPhaseStartConditions.
Creates the correct concrete EnemyPhaseStartConditions using the formatted string.
*/
class EnemySpawnConditionFactory {
public:
	static std::shared_ptr<LevelEventStartCondition> create(std::string formattedString);
};