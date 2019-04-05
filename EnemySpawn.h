#pragma once
#include <vector>
#include <utility>
#include <string>
#include <entt/entt.hpp>
#include "TextMarshallable.h"
#include "SpriteLoader.h"
#include "Constants.h"

class LevelPack;
class EntityCreationQueue;

class EnemySpawnInfo : public TextMarshallable {
public:
	inline EnemySpawnInfo() {}
	inline EnemySpawnInfo(int enemyID, float x, float y) : enemyID(enemyID), x(x), y(y) {}

	std::string format() override;
	void load(std::string formattedString) override;

	void spawnEnemy(SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue);

	inline float getX() { return x; }
	inline float getY() { return y; }
	inline int getHealthPacksOnDeath() { return healthPacksOnDeath; }
	inline int getPowerPacksOnDeath() { return powerPacksOnDeath; }
	inline int getPointPacksOnDeath() { return pointPacksOnDeath; }

private:
	int enemyID;
	float x;
	float y;

	// Item drops
	int healthPacksOnDeath = 0;
	int powerPacksOnDeath = 2;
	int pointPacksOnDeath = 6;
};