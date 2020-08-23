#include <LevelPack/EnemySpawn.h>

#include <LevelPack/LevelPack.h>
#include <LevelPack/EditorMovablePointSpawnType.h>
#include <Game/EntityCreationQueue.h>
#include <LevelPack/Item.h>

EnemySpawnInfo::EnemySpawnInfo() {
}

EnemySpawnInfo::EnemySpawnInfo(int enemyID, std::string x, std::string y, std::vector<std::pair<std::shared_ptr<Item>, std::string>> itemsDroppedOnDeath) : enemyID(enemyID), x(x), y(y), itemsDroppedOnDeath(itemsDroppedOnDeath) {
}

std::shared_ptr<LevelPackObject> EnemySpawnInfo::clone() const {
	auto clone = std::make_shared<EnemySpawnInfo>();
	clone->load(format());
	return clone;
}

std::string EnemySpawnInfo::format() const {
	std::string res = formatString(x) + formatString(y) + tos(enemyID) + formatTMObject(symbolTable) + formatTMObject(enemySymbolsDefiner);
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
	enemySymbolsDefiner.load(items[4]);
	itemsDroppedOnDeath.clear();
	for (int i = 5; i < items.size(); i += 2) {
		itemsDroppedOnDeath.push_back(std::make_pair(ItemFactory::create(items[i]), items[i + 1]));
	}
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EnemySpawnInfo::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(x, x)
	LEGAL_CHECK_EXPRESSION(y, y)
	
	// TODO: legal check itemsDroppedOnDeath
	int i = 0;
	for (auto p : itemsDroppedOnDeath) {
		auto itemLegal = p.first->legal(levelPack, spriteLoader, symbolTables);
		if (itemLegal.first != LEGAL_STATUS::LEGAL) {
			status = std::max(status, itemLegal.first);
			tabEveryLine(itemLegal.second);
			messages.push_back("Item drop index " + std::to_string(i) + ":");
			messages.insert(messages.end(), itemLegal.second.begin(), itemLegal.second.end());
		}

		if (!expressionStrIsValid(parser, p.second, symbolTables)) {
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
	compiledEnemySymbolsDefiner = enemySymbolsDefiner.toLowerLevelSymbolTable(expr);

	itemsDroppedOnDeathExprCompiledValue.clear();
	for (auto p : itemsDroppedOnDeath) {
		p.first->compileExpressions(symbolTables);
		parser.compile(p.second, expr);
		itemsDroppedOnDeathExprCompiledValue.push_back(std::make_pair(p.first, expr.value()));
	}
}

void EnemySpawnInfo::spawnEnemy(SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) {
	queue.pushBack(std::make_unique<SpawnEnemyCommand>(registry, spriteLoader, levelPack.getGameplayEnemy(enemyID, compiledEnemySymbolsDefiner), shared_from_this()));
}

int EnemySpawnInfo::getEnemyID() const {
	return enemyID;
}

float EnemySpawnInfo::getX() const {
	return xExprCompiledValue;
}

float EnemySpawnInfo::getY() const {
	return yExprCompiledValue;
}

const std::vector<std::pair<std::shared_ptr<Item>, std::string>> EnemySpawnInfo::getEditableItemsDroppedOnDeath() const {
	return itemsDroppedOnDeath;
}

const std::vector<std::pair<std::shared_ptr<Item>, int>> EnemySpawnInfo::getItemsDroppedOnDeath() const {
	return itemsDroppedOnDeathExprCompiledValue;
}

ExprSymbolTable EnemySpawnInfo::getEnemySymbolsDefiner() const {
	return enemySymbolsDefiner;
}

exprtk::symbol_table<float> EnemySpawnInfo::getCompiledEnemySymbolsDefiner() const {
	return compiledEnemySymbolsDefiner;
}

void EnemySpawnInfo::setEnemyID(int enemyID) {
	this->enemyID = enemyID;
}

void EnemySpawnInfo::setEnemySymbolsDefiner(ExprSymbolTable enemySymbolsDefiner) {
	this->enemySymbolsDefiner = enemySymbolsDefiner;
}

void EnemySpawnInfo::addItemDroppedOnDeath(std::pair<std::shared_ptr<Item>, std::string> itemAndAmount) {
	itemsDroppedOnDeath.push_back(itemAndAmount);
}
