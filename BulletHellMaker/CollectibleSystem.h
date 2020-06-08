#pragma once
#include <entt/entt.hpp>
#include "Components.h"
#include "EntityCreationQueue.h"
#include "SpatialHashTable.h"
#include "LevelPack.h"

/*
Manages collectibles
*/
class CollectibleSystem {
public:
	CollectibleSystem(EntityCreationQueue& queue, entt::DefaultRegistry& registry, const LevelPack& levelPack, float mapWidth, float mapHeight);
	void update(float deltaTime);

private:
	EntityCreationQueue& queue;
	entt::DefaultRegistry& registry;

	// Spatial hash table with cell size equal to largest item hitbox; contains all entities with CollectibleComponent inserted normally
	SpatialHashTable<uint32_t> itemHitboxTable;
	// Spatial hash table with cell size equal to largest item activation radius; contains all entities with CollectibleComponent inserted with hitbox radius equal to its item activation radius
	SpatialHashTable<uint32_t> activationTable;

	inline float distance(float x1, float y1, float x2, float y2) {
		return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
	}

	inline bool collides(const PositionComponent& p1, const HitboxComponent& h1, const PositionComponent& p2, float h2x, float h2y, float h2radius) {
		return distance(p1.getX() + h1.getX(), p1.getY() + h1.getY(), p2.getX() + h2x, p2.getY() + h2y) <= (h1.getRadius() + h2radius);
	}

	inline bool collides(const PositionComponent& p1, const HitboxComponent& h1, const PositionComponent& p2, const HitboxComponent& h2) {
		return distance(p1.getX() + h1.getX(), p1.getY() + h1.getY(), p2.getX() + h2.getX(), p2.getY() + h2.getY()) <= (h1.getRadius() + h2.getRadius());
	}
};