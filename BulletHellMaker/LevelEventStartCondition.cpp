#include "LevelEventStartCondition.h"
#include "Components.h"

std::shared_ptr<LevelPackObject> GlobalTimeBasedEnemySpawnCondition::clone() const {
	auto clone = std::make_shared<GlobalTimeBasedEnemySpawnCondition>();
	clone->load(format());
	return clone;
}

std::string GlobalTimeBasedEnemySpawnCondition::format() const {
	return formatString("GlobalTimeBasedEnemySpawnCondition") + formatString(time);
}

void GlobalTimeBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	time = std::stof(items[1]);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> GlobalTimeBasedEnemySpawnCondition::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	exprtk::parser<float> parser;
	if (!expressionStrIsValid(parser, time, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for time");
	}
	return std::make_pair(status, messages);
}

void GlobalTimeBasedEnemySpawnCondition::compileExpressions(exprtk::symbol_table<float> symbolTable) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(time)
}

bool GlobalTimeBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.get<LevelManagerTag>().getTimeSinceStartOfLevel() >= timeExprCompiledValue;
}

std::shared_ptr<LevelPackObject> EnemyCountBasedEnemySpawnCondition::clone() const {
	auto clone = std::make_shared<EnemyCountBasedEnemySpawnCondition>();
	clone->load(format());
	return clone;
}

std::string EnemyCountBasedEnemySpawnCondition::format() const {
	return formatString("EnemyCountBasedEnemySpawnCondition") + formatString(enemyCount);
}

void EnemyCountBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	enemyCount = std::stoi(items[1]);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EnemyCountBasedEnemySpawnCondition::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	exprtk::parser<float> parser;
	if (!expressionStrIsValid(parser, enemyCount, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for enemy count");
	}
	return std::make_pair(status, messages);
}

void EnemyCountBasedEnemySpawnCondition::compileExpressions(exprtk::symbol_table<float> symbolTable) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_INT(enemyCount)
}

bool EnemyCountBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.view<EnemyComponent>().size() - 1 <= enemyCountExprCompiledValue;
}

std::shared_ptr<LevelPackObject> TimeBasedEnemySpawnCondition::clone() const {
	auto clone = std::make_shared<TimeBasedEnemySpawnCondition>();
	clone->load(format());
	return clone;
}

std::string TimeBasedEnemySpawnCondition::format() const {
	return formatString("TimeBasedEnemySpawnCondition") + formatString(time);
}

void TimeBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	time = std::stof(items[1]);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> TimeBasedEnemySpawnCondition::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	exprtk::parser<float> parser;
	if (!expressionStrIsValid(parser, time, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for time");
	}
	return std::make_pair(status, messages);
}

void TimeBasedEnemySpawnCondition::compileExpressions(exprtk::symbol_table<float> symbolTable) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(time)
}

bool TimeBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.get<LevelManagerTag>().getTimeSinceLastEnemySpawn() >= timeExprCompiledValue;
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
