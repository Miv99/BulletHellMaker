#include "EnemySpawn.h"
#include "LevelPack.h"
#include "Components.h"
#include "EditorMovablePointSpawnType.h"
#include "EntityCreationQueue.h"
#include "Item.h"

EnemySpawnInfo::EnemySpawnInfo(int enemyID, std::string x, std::string y, std::vector<std::pair<std::shared_ptr<Item>, std::string>> itemsDroppedOnDeath) : enemyID(enemyID), x(x), y(y), itemsDroppedOnDeath(itemsDroppedOnDeath) {
}

std::shared_ptr<LevelPackObject> EnemySpawnInfo::clone() const {
	auto clone = std::make_shared<EnemySpawnInfo>();
	clone->load(format());
	return clone;
}

std::string EnemySpawnInfo::format() const {
	std::string res = formatString(x) + formatString(y) + tos(enemyID) + formatTMObject(symbolTable);
	for (auto pair : itemsDroppedOnDeath) {
		res += formatTMObject(*pair.first) + formatString(pair.second);
	}
	return res;
}

void EnemySpawnInfo::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = items[0];
	y = items[1];
	enemyID = std::stoi(items[2]);
	symbolTable.load(items[3]);
	itemsDroppedOnDeath.clear();
	for (int i = 4; i < items.size(); i += 2) {
		itemsDroppedOnDeath.push_back(std::make_pair(ItemFactory::create(items[i]), items[i + 1]));
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
	int i = 0;
	for (auto p : itemsDroppedOnDeath) {
		auto itemLegal = p.first->legal(levelPack, spriteLoader);
		if (itemLegal.first != LEGAL_STATUS::LEGAL) {
			status = std::max(status, itemLegal.first);
			tabEveryLine(itemLegal.second);
			messages.push_back("Item drop index " + std::to_string(i) + ":");
			messages.insert(messages.end(), itemLegal.second.begin(), itemLegal.second.end());
		}

		if (!expressionStrIsValid(parser, p.second, symbolTable)) {
			status = std::max(status, LEGAL_STATUS::ILLEGAL);
			messages.push_back("Invalid expression for item drop index " + std::to_string(i) + " amount");
		}
	}
	return std::make_pair(status, messages);
}

void EnemySpawnInfo::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(x)
	COMPILE_EXPRESSION_FOR_FLOAT(y)

	itemsDroppedOnDeathExprCompiledValue.clear();
	for (auto p : itemsDroppedOnDeath) {
		p.first->compileExpressions(symbolTables);
		parser.compile(p.second, expr);
		itemsDroppedOnDeathExprCompiledValue.push_back(std::make_pair(p.first, expr.value()));
	}
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

const std::vector<std::pair<std::shared_ptr<Item>, std::string>> EnemySpawnInfo::getEditableItemsDroppedOnDeath() {
	return itemsDroppedOnDeath;
}

const std::vector<std::pair<std::shared_ptr<Item>, int>> EnemySpawnInfo::getItemsDroppedOnDeath() {
	return itemsDroppedOnDeathExprCompiledValue;
}

void EnemySpawnInfo::addItemDroppedOnDeath(std::pair<std::shared_ptr<Item>, std::string> itemAndAmount) {
	itemsDroppedOnDeath.push_back(itemAndAmount);
}
