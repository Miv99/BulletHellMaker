#pragma once
#include <vector>
#include <utility>
#include <string>
#include <entt/entt.hpp>
#include "TextMarshallable.h"
#include "SpriteLoader.h"

class LevelPack;
class EntityCreationQueue;

class EnemySpawnInfo : public TextMarshallable {
public:
	inline EnemySpawnInfo() {}
	inline EnemySpawnInfo(int enemyID, float x, float y) : enemyID(enemyID), x(x), y(y) {}

	std::string format() override;
	void load(std::string formattedString) override;

	void spawnEnemy(SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue);

private:
	int enemyID;
	float x;
	float y;
};