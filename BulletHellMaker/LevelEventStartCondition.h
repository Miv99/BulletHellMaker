#pragma once
#include <vector>
#include <utility>
#include <string>
#include <entt/entt.hpp>
#include "TextMarshallable.h"
#include "LevelPackObject.h"
#include "ExpressionCompilable.h"

class LevelEventStartCondition : public TextMarshallable, public LevelPackObject, public ExpressionCompilable {
public:
	virtual std::shared_ptr<LevelPackObject> clone() const = 0;

	std::string format() const = 0;
	void load(std::string formattedString) = 0;

	virtual std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const = 0;
	virtual void compileExpressions(exprtk::symbol_table<float> symbolTable) = 0;

	virtual bool satisfied(entt::DefaultRegistry& registry) = 0;
};

/*
LevelEventStartCondition that depends on the time since the start of the level.
*/
class GlobalTimeBasedEnemySpawnCondition : public LevelEventStartCondition {
public:
	inline GlobalTimeBasedEnemySpawnCondition() {}
	inline GlobalTimeBasedEnemySpawnCondition(std::string time) : time(time) {}

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const override;
	void compileExpressions(exprtk::symbol_table<float> symbolTable) override;

	bool satisfied(entt::DefaultRegistry& registry) override;

private:
	// Minimum time since the start of the level for this condition to be satisfied
	std::string time;
	exprtk::expression<float> timeExpr;
	float timeExprCompiledValue;
};

/*
LevelEventStartCondition that depends on the time since the last enemy spawn.
*/
class TimeBasedEnemySpawnCondition : public LevelEventStartCondition {
public:
	inline TimeBasedEnemySpawnCondition() {}
	inline TimeBasedEnemySpawnCondition(std::string time) : time(time) {}

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const override;
	void compileExpressions(exprtk::symbol_table<float> symbolTable) override;

	bool satisfied(entt::DefaultRegistry& registry) override;

private:
	// Minimum time since the last enemy's spawn for this condition to be satisfied
	std::string time;
	exprtk::expression<float> timeExpr;
	float timeExprCompiledValue;
};

/*
LevelEventStartCondition that depends on the number of enemies alive.
*/
class EnemyCountBasedEnemySpawnCondition : public LevelEventStartCondition {
public:
	inline EnemyCountBasedEnemySpawnCondition() {}
	inline EnemyCountBasedEnemySpawnCondition(std::string enemyCount) : enemyCount(enemyCount) {}

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const override;
	void compileExpressions(exprtk::symbol_table<float> symbolTable) override;

	bool satisfied(entt::DefaultRegistry& registry) override;

private:
	// Maximum number of other enemies alive for this condition to be satisfied
	std::string enemyCount;
	exprtk::expression<float> enemyCountExpr;
	int enemyCountExprCompiledValue;
};

/*
The factory for creating EnemyPhaseStartConditions.
Creates the correct concrete EnemyPhaseStartConditions using the formatted string.
*/
class LevelEventStartConditionFactory {
public:
	static std::shared_ptr<LevelEventStartCondition> create(std::string formattedString);
};