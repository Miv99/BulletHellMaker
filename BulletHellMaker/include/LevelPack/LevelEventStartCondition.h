#pragma once
#include <vector>
#include <utility>
#include <string>

#include <entt/entt.hpp>

#include <LevelPack/TextMarshallable.h>
#include <LevelPack/LevelPackObject.h>
#include <LevelPack/ExpressionCompilable.h>

class LevelEventStartCondition : public LevelPackObject {
public:
	virtual std::shared_ptr<LevelPackObject> clone() const = 0;

	std::string format() const = 0;
	void load(std::string formattedString) = 0;

	virtual std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const = 0;
	virtual void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) = 0;

	virtual bool satisfied(entt::DefaultRegistry& registry) = 0;
};

/*
LevelEventStartCondition that depends on the time since the start of the level.
*/
class GlobalTimeBasedEnemySpawnCondition : public LevelEventStartCondition {
public:
	GlobalTimeBasedEnemySpawnCondition();
	GlobalTimeBasedEnemySpawnCondition(std::string time);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	bool satisfied(entt::DefaultRegistry& registry) override;

private:
	// Minimum time since the start of the level for this condition to be satisfied
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(time, float, 0)
};

/*
LevelEventStartCondition that depends on the time since the last enemy spawn.
*/
class TimeBasedEnemySpawnCondition : public LevelEventStartCondition {
public:
	TimeBasedEnemySpawnCondition();
	TimeBasedEnemySpawnCondition(std::string time);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	bool satisfied(entt::DefaultRegistry& registry) override;

private:
	// Minimum time since the last enemy's spawn for this condition to be satisfied
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(time, float, 0)
};

/*
LevelEventStartCondition that depends on the number of enemies alive.
*/
class EnemyCountBasedEnemySpawnCondition : public LevelEventStartCondition {
public:
	EnemyCountBasedEnemySpawnCondition();
	EnemyCountBasedEnemySpawnCondition(std::string enemyCount);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	bool satisfied(entt::DefaultRegistry& registry) override;

private:
	// Maximum number of other enemies alive for this condition to be satisfied
	DEFINE_EXPRESSION_VARIABLE(enemyCount, int)
};

/*
The factory for creating EnemyPhaseStartConditions.
Creates the correct concrete EnemyPhaseStartConditions using the formatted string.
*/
class LevelEventStartConditionFactory {
public:
	static std::shared_ptr<LevelEventStartCondition> create(std::string formattedString);
};