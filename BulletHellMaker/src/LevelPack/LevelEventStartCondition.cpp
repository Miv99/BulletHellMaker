#include <LevelPack/LevelEventStartCondition.h>

#include <Game/Components/EnemyComponent.h>
#include <Game/Components/LevelManagerTag.h>

GlobalTimeBasedEnemySpawnCondition::GlobalTimeBasedEnemySpawnCondition() {
}

GlobalTimeBasedEnemySpawnCondition::GlobalTimeBasedEnemySpawnCondition(std::string time) : time(time) {
}

std::shared_ptr<LevelPackObject> GlobalTimeBasedEnemySpawnCondition::clone() const {
	auto clone = std::make_shared<GlobalTimeBasedEnemySpawnCondition>();
	clone->load(format());
	return clone;
}

std::string GlobalTimeBasedEnemySpawnCondition::format() const {
	return formatString("GlobalTimeBasedEnemySpawnCondition") + formatString(time) + formatTMObject(symbolTable);
}

void GlobalTimeBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	time = items.at(1);
	symbolTable.load(items.at(2));
}

nlohmann::json GlobalTimeBasedEnemySpawnCondition::toJson() {
	return {
		{"className", "GlobalTimeBasedEnemySpawnCondition"},
		{"valueSymbolTable", symbolTable.toJson()},
		{"time", time}
	};
}

void GlobalTimeBasedEnemySpawnCondition::load(const nlohmann::json& j) {
	if (j.contains("valueSymbolTable")) {
		symbolTable.load(j.at("valueSymbolTable"));
	} else {
		symbolTable = ValueSymbolTable();
	}

	j.at("time").get_to(time);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> GlobalTimeBasedEnemySpawnCondition::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(time, time)

	return std::make_pair(status, messages);
}

void GlobalTimeBasedEnemySpawnCondition::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(time)
}

bool GlobalTimeBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.get<LevelManagerTag>().getTimeSinceStartOfLevel() >= timeExprCompiledValue;
}

EnemyCountBasedEnemySpawnCondition::EnemyCountBasedEnemySpawnCondition() {
}

EnemyCountBasedEnemySpawnCondition::EnemyCountBasedEnemySpawnCondition(std::string enemyCount)
	: enemyCount(enemyCount) {
}

std::shared_ptr<LevelPackObject> EnemyCountBasedEnemySpawnCondition::clone() const {
	auto clone = std::make_shared<EnemyCountBasedEnemySpawnCondition>();
	clone->load(format());
	return clone;
}

std::string EnemyCountBasedEnemySpawnCondition::format() const {
	return formatString("EnemyCountBasedEnemySpawnCondition") + formatString(enemyCount) + formatTMObject(symbolTable);
}

void EnemyCountBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	enemyCount = items.at(1);
	symbolTable.load(items.at(2));
}

nlohmann::json EnemyCountBasedEnemySpawnCondition::toJson() {
	return {
		{"className", "EnemyCountBasedEnemySpawnCondition"},
		{"valueSymbolTable", symbolTable.toJson()},
		{"enemyCount", enemyCount}
	};
}

void EnemyCountBasedEnemySpawnCondition::load(const nlohmann::json& j) {
	if (j.contains("valueSymbolTable")) {
		symbolTable.load(j.at("valueSymbolTable"));
	} else {
		symbolTable = ValueSymbolTable();
	}

	j.at("enemyCount").get_to(enemyCount);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EnemyCountBasedEnemySpawnCondition::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(enemyCount, enemy count)

	return std::make_pair(status, messages);
}

void EnemyCountBasedEnemySpawnCondition::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_INT(enemyCount)
}

bool EnemyCountBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.view<EnemyComponent>().size() - 1 <= enemyCountExprCompiledValue;
}

TimeBasedEnemySpawnCondition::TimeBasedEnemySpawnCondition() {
}

TimeBasedEnemySpawnCondition::TimeBasedEnemySpawnCondition(std::string time) 
	: time(time) {
}

std::shared_ptr<LevelPackObject> TimeBasedEnemySpawnCondition::clone() const {
	auto clone = std::make_shared<TimeBasedEnemySpawnCondition>();
	clone->load(format());
	return clone;
}

std::string TimeBasedEnemySpawnCondition::format() const {
	return formatString("TimeBasedEnemySpawnCondition") + formatString(time) + formatTMObject(symbolTable);
}

void TimeBasedEnemySpawnCondition::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	time = items.at(1);
	symbolTable.load(items.at(2));
}

nlohmann::json TimeBasedEnemySpawnCondition::toJson() {
	return {
		{"className", "TimeBasedEnemySpawnCondition"},
		{"valueSymbolTable", symbolTable.toJson()},
		{"time", time}
	};
}

void TimeBasedEnemySpawnCondition::load(const nlohmann::json& j) {
	if (j.contains("valueSymbolTable")) {
		symbolTable.load(j.at("valueSymbolTable"));
	} else {
		symbolTable = ValueSymbolTable();
	}

	j.at("time").get_to(time);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> TimeBasedEnemySpawnCondition::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(time, time)

	return std::make_pair(status, messages);
}

void TimeBasedEnemySpawnCondition::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(time)
}

bool TimeBasedEnemySpawnCondition::satisfied(entt::DefaultRegistry & registry) {
	return registry.get<LevelManagerTag>().getTimeSinceLastEnemySpawn() >= timeExprCompiledValue;
}

std::shared_ptr<LevelEventStartCondition> LevelEventStartConditionFactory::create(std::string formattedString) {
	auto name = split(formattedString, TextMarshallable::DELIMITER)[0];
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

std::shared_ptr<LevelEventStartCondition> LevelEventStartConditionFactory::create(const nlohmann::json& j) {
	if (j.contains("className")) {
		std::string name;
		j.at("className").get_to(name);

		std::shared_ptr<LevelEventStartCondition> ptr;
		if (name == "GlobalTimeBasedEnemySpawnCondition") {
			ptr = std::make_shared<GlobalTimeBasedEnemySpawnCondition>();
		} else if (name == "TimeBasedEnemySpawnCondition") {
			ptr = std::make_shared<TimeBasedEnemySpawnCondition>();
		} else if (name == "EnemyCountBasedEnemySpawnCondition") {
			ptr = std::make_shared<EnemyCountBasedEnemySpawnCondition>();
		}
		ptr->load(j);
		return ptr;
	} else {
		return std::make_shared<GlobalTimeBasedEnemySpawnCondition>();
	}
}
