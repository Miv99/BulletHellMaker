#pragma once
#include <vector>
#include <utility>
#include <string>
#include <memory>
#include <entt/entt.hpp>
#include "TextMarshallable.h"
#include "SpriteLoader.h"
#include "Constants.h"
#include "LevelPackObject.h"
#include "SymbolTable.h"
#include "ExpressionCompilable.h"

class LevelPack;
class EntityCreationQueue;
class Item;

class EnemySpawnInfo : public LevelPackObject, public std::enable_shared_from_this<EnemySpawnInfo> {
public:
	inline EnemySpawnInfo() {}
	EnemySpawnInfo(int enemyID, std::string x, std::string y, std::vector<std::pair<std::shared_ptr<Item>, std::string>> itemsDroppedOnDeath);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	void spawnEnemy(SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue);

	int getEnemyID() const;
	float getX() const;
	float getY() const;
	const std::vector<std::pair<std::shared_ptr<Item>, std::string>> getEditableItemsDroppedOnDeath() const;
	const std::vector<std::pair<std::shared_ptr<Item>, int>> getItemsDroppedOnDeath() const;
	ExprSymbolTable getEnemySymbolsDefiner() const;
	exprtk::symbol_table<float> getCompiledEnemySymbolsDefiner() const;

	void setEnemyID(int enemyID);
	void setEnemySymbolsDefiner(ExprSymbolTable enemySymbolsDefiner);

	void addItemDroppedOnDeath(std::pair<std::shared_ptr<Item>, std::string> itemAndAmount);
	
private:
	int enemyID;
	ExprSymbolTable enemySymbolsDefiner;
	exprtk::symbol_table<float> compiledEnemySymbolsDefiner;

	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(x, float, 0)
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(y, float, 0)

	// Items dropped and their amount
	std::vector<std::pair<std::shared_ptr<Item>, std::string>> itemsDroppedOnDeath;
	std::vector<std::pair<std::shared_ptr<Item>, int>> itemsDroppedOnDeathExprCompiledValue;
};