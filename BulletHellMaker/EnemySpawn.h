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

class LevelPack;
class EntityCreationQueue;
class Item;

class EnemySpawnInfo : public TextMarshallable, public LevelPackObject, public std::enable_shared_from_this<EnemySpawnInfo> {
public:
	inline EnemySpawnInfo() {}
	EnemySpawnInfo(int enemyID, std::string x, std::string y, std::vector<std::pair<std::shared_ptr<Item>, int>> itemsDroppedOnDeath);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<bool, std::string> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const override;
	void compileExpressions(exprtk::symbol_table<float> symbolTable) override;

	void spawnEnemy(SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue);

	inline int getEnemyID() const { return enemyID; }
	float getX();
	float getY();
	const std::vector<std::pair<std::shared_ptr<Item>, int>> getItemsDroppedOnDeath();

	void addItemDroppedOnDeath(std::pair<std::shared_ptr<Item>, int> itemAndAmount);
	
private:
	int enemyID;
	std::string x;
	std::string y;
	exprtk::expression<float> xExpr;
	exprtk::expression<float> yExpr;

	ValueSymbolTable symbolTable;

	// Items dropped and their amount
	std::vector<std::pair<std::shared_ptr<Item>, int>> itemsDroppedOnDeath;
};