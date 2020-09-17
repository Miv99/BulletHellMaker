#include <LevelPack/EnemyPhaseStartCondition.h>

#include <Game/Components/HealthComponent.h>
#include <Game/Components/EnemyComponent.h>

TimeBasedEnemyPhaseStartCondition::TimeBasedEnemyPhaseStartCondition() {
}

TimeBasedEnemyPhaseStartCondition::TimeBasedEnemyPhaseStartCondition(std::string time) 
	: time(time) {
}

std::shared_ptr<LevelPackObject> TimeBasedEnemyPhaseStartCondition::clone() const {
	auto clone = std::make_shared<TimeBasedEnemyPhaseStartCondition>();
	clone->load(format());
	return clone;
}

std::string TimeBasedEnemyPhaseStartCondition::format() const {
	return formatString("TimeBasedEnemyPhaseStartCondition") + formatString(time) + formatTMObject(symbolTable);
}

void TimeBasedEnemyPhaseStartCondition::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	time = items.at(1);
	symbolTable.load(items.at(2));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> TimeBasedEnemyPhaseStartCondition::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(time, time)

	return std::make_pair(status, messages);
}

void TimeBasedEnemyPhaseStartCondition::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(time)
}

bool TimeBasedEnemyPhaseStartCondition::satisfied(entt::DefaultRegistry & registry, uint32_t entity) {
	return registry.get<EnemyComponent>(entity).getTimeSinceLastPhase() >= timeExprCompiledValue;
}

float TimeBasedEnemyPhaseStartCondition::getTime() {
	return timeExprCompiledValue;
}

void TimeBasedEnemyPhaseStartCondition::setTime(std::string time) {
	this->time = time;
}

HPBasedEnemyPhaseStartCondition::HPBasedEnemyPhaseStartCondition() {
}

HPBasedEnemyPhaseStartCondition::HPBasedEnemyPhaseStartCondition(std::string ratio) : ratio(ratio) {
}

std::shared_ptr<LevelPackObject> HPBasedEnemyPhaseStartCondition::clone() const {
	auto clone = std::make_shared<HPBasedEnemyPhaseStartCondition>();
	clone->load(format());
	return clone;
}

std::string HPBasedEnemyPhaseStartCondition::format() const {
	return formatString("HPBasedEnemyPhaseStartCondition") + formatString(ratio) + formatTMObject(symbolTable);
}

void HPBasedEnemyPhaseStartCondition::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	ratio = items.at(1);
	symbolTable.load(items.at(2));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> HPBasedEnemyPhaseStartCondition::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(ratio, max health ratio)

	return std::make_pair(status, messages);
}

void HPBasedEnemyPhaseStartCondition::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(ratio)
}

bool HPBasedEnemyPhaseStartCondition::satisfied(entt::DefaultRegistry & registry, uint32_t entity) {
	auto& health = registry.get<HealthComponent>(entity);
	return health.getHealth()/health.getMaxHealth() <= ratioExprCompiledValue;
}

float HPBasedEnemyPhaseStartCondition::getRatio() {
	return ratioExprCompiledValue;
}

void HPBasedEnemyPhaseStartCondition::setRatio(std::string ratio) {
	this->ratio = ratio;
}

EnemyCountBasedEnemyPhaseStartCondition::EnemyCountBasedEnemyPhaseStartCondition() {
}

EnemyCountBasedEnemyPhaseStartCondition::EnemyCountBasedEnemyPhaseStartCondition(std::string enemyCount) 
	: enemyCount(enemyCount) {
}

std::shared_ptr<LevelPackObject> EnemyCountBasedEnemyPhaseStartCondition::clone() const {
	auto clone = std::make_shared<EnemyCountBasedEnemyPhaseStartCondition>();
	clone->load(format());
	return clone;
}

std::string EnemyCountBasedEnemyPhaseStartCondition::format() const {
	return formatString("EnemyCountBasedEnemyPhaseStartCondition") + formatString(enemyCount) + formatTMObject(symbolTable);
}

void EnemyCountBasedEnemyPhaseStartCondition::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	enemyCount = items.at(1);
	symbolTable.load(items.at(2));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EnemyCountBasedEnemyPhaseStartCondition::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(enemyCount, max enemy count)

	return std::make_pair(status, messages);
}

void EnemyCountBasedEnemyPhaseStartCondition::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_INT(enemyCount)
}

bool EnemyCountBasedEnemyPhaseStartCondition::satisfied(entt::DefaultRegistry & registry, uint32_t entity) {
	return registry.view<EnemyComponent>().size() - 1 <= enemyCountExprCompiledValue;
}

int EnemyCountBasedEnemyPhaseStartCondition::getEnemyCount() {
	return enemyCountExprCompiledValue;
}

void EnemyCountBasedEnemyPhaseStartCondition::setEnemyCount(std::string enemyCount) {
	this->enemyCount = enemyCount;
}

std::shared_ptr<EnemyPhaseStartCondition> EnemyPhaseStartConditionFactory::create(std::string formattedString) {
	auto name = split(formattedString, TextMarshallable::DELIMITER)[0];
	std::shared_ptr<EnemyPhaseStartCondition> ptr;
	if (name == "TimeBasedEnemyPhaseStartCondition") {
		ptr = std::make_shared<TimeBasedEnemyPhaseStartCondition>();
	} else if (name == "HPBasedEnemyPhaseStartCondition") {
		ptr = std::make_shared<HPBasedEnemyPhaseStartCondition>();
	} else if (name == "EnemyCountBasedEnemyPhaseStartCondition") {
		ptr = std::make_shared<EnemyCountBasedEnemyPhaseStartCondition>();
	}
	ptr->load(formattedString);
	return ptr;
}