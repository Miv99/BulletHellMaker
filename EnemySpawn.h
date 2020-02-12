#pragma once
#include <vector>
#include <utility>
#include <string>
#include <memory>
#include <entt/entt.hpp>
#include "TextMarshallable.h"
#include "SpriteLoader.h"
#include "Constants.h"

class LevelPack;
class EntityCreationQueue;
class Item;

class EnemySpawnInfo : public TextMarshallable {
public:
	inline EnemySpawnInfo() {}
	EnemySpawnInfo(int enemyID, float x, float y, std::vector<std::pair<std::shared_ptr<Item>, int>> itemsDroppedOnDeath);

	std::string format() const override;
	void load(std::string formattedString) override;

	void spawnEnemy(SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue);

	inline int getEnemyID() const { return enemyID; }
	inline float getX() const { return x; }
	inline float getY() const { return y; }
	const std::vector<std::pair<std::shared_ptr<Item>, int>> getItemsDroppedOnDeath();

	void addItemDroppedOnDeath(std::pair<std::shared_ptr<Item>, int> itemAndAmount);
	
private:
	int enemyID;
	float x;
	float y;

	// Items dropped and their amount
	std::vector<std::pair<std::shared_ptr<Item>, int>> itemsDroppedOnDeath;
};