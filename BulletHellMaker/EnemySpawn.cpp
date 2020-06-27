#include "EnemySpawn.h"
#include "LevelPack.h"
#include "Components.h"
#include "EditorMovablePointSpawnType.h"
#include "EntityCreationQueue.h"
#include "Item.h"

EnemySpawnInfo::EnemySpawnInfo(int enemyID, std::string x, std::string y, std::vector<std::pair<std::shared_ptr<Item>, int>> itemsDroppedOnDeath) : enemyID(enemyID), x(x), y(y), itemsDroppedOnDeath(itemsDroppedOnDeath) {
}

std::shared_ptr<LevelPackObject> EnemySpawnInfo::clone() const {
	auto clone = std::make_shared<EnemySpawnInfo>();
	clone->load(format());
	return clone;
}

std::string EnemySpawnInfo::format() const {
	std::string res = formatString(x) + formatString(y) + tos(enemyID);
	for (auto pair : itemsDroppedOnDeath) {
		res += formatTMObject(*pair.first) + tos(pair.second);
	}
	return res;
}

void EnemySpawnInfo::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = items[0];
	y = items[1];
	enemyID = std::stoi(items[2]);
	itemsDroppedOnDeath.clear();
	for (int i = 3; i < items.size(); i += 2) {
		itemsDroppedOnDeath.push_back(std::make_pair(ItemFactory::create(items[i]), std::stoi(items[i + 1])));
	}
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EnemySpawnInfo::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	exprtk::parser<float> parser;
	if (!expressionStrIsValid(parser, x, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for x");
	}
	if (!expressionStrIsValid(parser, y, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for y");
	}
	// TODO: legal check itemsDroppedOnDeath
	return std::make_pair(status, messages);
}

void EnemySpawnInfo::compileExpressions(exprtk::symbol_table<float> symbolTable) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(x)
	COMPILE_EXPRESSION_FOR_FLOAT(y)
}

void EnemySpawnInfo::spawnEnemy(SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) {
	queue.pushBack(std::make_unique<SpawnEnemyCommand>(registry, spriteLoader, levelPack.getEnemy(enemyID), shared_from_this()));
}

float EnemySpawnInfo::getX() {
	return xExprCompiledValue;
}

float EnemySpawnInfo::getY() {
	return yExprCompiledValue;
}

const std::vector<std::pair<std::shared_ptr<Item>, int>> EnemySpawnInfo::getItemsDroppedOnDeath() {
	return itemsDroppedOnDeath;
}

void EnemySpawnInfo::addItemDroppedOnDeath(std::pair<std::shared_ptr<Item>, int> itemAndAmount) {
	itemsDroppedOnDeath.push_back(itemAndAmount);
}
