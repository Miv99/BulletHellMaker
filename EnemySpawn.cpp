#include "EnemySpawn.h"
#include "LevelPack.h"
#include "Components.h"
#include "EditorMovablePointSpawnType.h"
#include "EntityCreationQueue.h"
#include "Item.h"

EnemySpawnInfo::EnemySpawnInfo(int enemyID, float x, float y, std::vector<std::pair<std::shared_ptr<Item>, int>> itemsDroppedOnDeath) : enemyID(enemyID), x(x), y(y), itemsDroppedOnDeath(itemsDroppedOnDeath) {
}

std::string EnemySpawnInfo::format() const {
	std::string res = tos(x) + tos(y) + tos(enemyID);
	for (auto pair : itemsDroppedOnDeath) {
		res += formatTMObject(*pair.first) + tos(pair.second);
	}
	return res;
}

void EnemySpawnInfo::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = std::stof(items[0]);
	y = std::stof(items[1]);
	enemyID = std::stoi(items[2]);
	for (int i = 3; i < items.size(); i += 2) {
		itemsDroppedOnDeath.push_back(std::make_pair(ItemFactory::create(items[i]), std::stoi(items[i + 1])));
	}
}

void EnemySpawnInfo::spawnEnemy(SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) {
	queue.pushBack(std::make_unique<SpawnEnemyCommand>(registry, spriteLoader, levelPack.getEnemy(enemyID), *this));
}

const std::vector<std::pair<std::shared_ptr<Item>, int>> EnemySpawnInfo::getItemsDroppedOnDeath() {
	return itemsDroppedOnDeath;
}

void EnemySpawnInfo::addItemDroppedOnDeath(std::pair<std::shared_ptr<Item>, int> itemAndAmount) {
	itemsDroppedOnDeath.push_back(itemAndAmount);
}
