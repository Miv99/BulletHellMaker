#pragma once
#include <string>
#include <vector>
#include <utility>
#include <memory>

#include <entt/entt.hpp>

#include <LevelPack/LevelPackObject.h>
#include <LevelPack/EnemyPhaseAction.h>

/*
The condition for an enemy phase to start.
*/
class EnemyPhaseStartCondition : public LevelPackObject {
public:
	virtual std::shared_ptr<LevelPackObject> clone() const = 0;

	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;

	virtual std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const = 0;
	virtual void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) = 0;

	virtual bool satisfied(entt::DefaultRegistry& registry, uint32_t entity) = 0;
};

/*
EnemyPhaseStartCondition that depends on the time since the enemy's last phase.
*/
class TimeBasedEnemyPhaseStartCondition : public EnemyPhaseStartCondition {
public:
	inline TimeBasedEnemyPhaseStartCondition() {}
	inline TimeBasedEnemyPhaseStartCondition(std::string time) : time(time) {}

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	bool satisfied(entt::DefaultRegistry& registry, uint32_t entity) override;

	float getTime();

	void setTime(std::string time);

private:
	// Minimum time since the start of the enemy's last phase for this condition to be satisfied
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(time, float, 30)
};

/*
EnemyPhaseStartCondition that depends on the enemy's percent health remaining.
*/
class HPBasedEnemyPhaseStartCondition : public EnemyPhaseStartCondition {
public:
	inline HPBasedEnemyPhaseStartCondition() {}
	inline HPBasedEnemyPhaseStartCondition(std::string ratio) : ratio(ratio) {}

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	bool satisfied(entt::DefaultRegistry& registry, uint32_t entity) override;

	float getRatio();

	void setRatio(std::string ratio);

private:
	// Maximum hp ratio of the enemy for this condition to be satisfied; range (0, 1]
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(ratio, float, 1)
};

/*
EnemyPhaseStartCondition that depends on the number of other enemies alive.
The enemy that this condition belongs to does not count.
*/
class EnemyCountBasedEnemyPhaseStartCondition : public EnemyPhaseStartCondition {
public:
	inline EnemyCountBasedEnemyPhaseStartCondition() {}
	inline EnemyCountBasedEnemyPhaseStartCondition(std::string enemyCount) : enemyCount(enemyCount) {}

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	bool satisfied(entt::DefaultRegistry& registry, uint32_t entity) override;

	int getEnemyCount();
	void setEnemyCount(std::string enemyCount);

private:
	// Maximum number of other enemies alive for this condition to be satisfied
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(enemyCount, int, 0)
};

/*
The factory for creating EnemyPhaseStartConditions.
Creates the correct concrete EnemyPhaseStartConditions using the formatted string.
*/
class EnemyPhaseStartConditionFactory {
public:
	static std::shared_ptr<EnemyPhaseStartCondition> create(std::string formattedString);
};