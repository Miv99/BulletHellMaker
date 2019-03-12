#include "EnemySpawn.h"
#include "LevelPack.h"
#include "Components.h"
#include "EditorMovablePointSpawnType.h"
#include "EntityCreationQueue.h"

std::string EnemySpawnInfo::format() {
	std::string res = "";
	res += tos(x) + delim;
	res += tos(y) + delim;
	res += tos(enemyID);
	return res;
}

void EnemySpawnInfo::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = std::stof(items[0]);
	y = std::stof(items[1]);
	enemyID = std::stoi(items[2]);
}

void EnemySpawnInfo::spawnEnemy(SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) {
	queue.addCommand(std::make_unique<SpawnEnemyCommand>(registry, spriteLoader, levelPack.getEnemy(enemyID), x, y));
}
