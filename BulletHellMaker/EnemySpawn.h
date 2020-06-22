#pragma once
#include <vector>
#include <utility>
#include <string>
#include <memory>
#include <entt/entt.hpp>
#include "TextMarshallable.h"
#include "SpriteLoader.h"
#include "Constants.h"
#include "exprtk.hpp"

class LevelPack;
class EntityCreationQueue;
class Item;

class EnemySpawnInfo : public TextMarshallable, public std::enable_shared_from_this<EnemySpawnInfo> {
public:
	inline EnemySpawnInfo() {}
	EnemySpawnInfo(int enemyID, std::string x, std::string y, std::vector<std::pair<std::shared_ptr<Item>, int>> itemsDroppedOnDeath);

	std::string format() const override;
	void load(std::string formattedString) override;

	void compileExpressions(exprtk::symbol_table<float> symbolTable);

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

	// Items dropped and their amount
	std::vector<std::pair<std::shared_ptr<Item>, int>> itemsDroppedOnDeath;
};