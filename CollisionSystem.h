#pragma once
#include <entt/entt.hpp>
#include "SpatialHashTable.h"
#include "Components.h"
#include "SpriteLoader.h"

class CollisionSystem {
public:
	CollisionSystem(SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, float mapWidth, float mapHeight, const HitboxComponent& largestHitbox);
	void update(float deltaTime);

private:
	SpriteLoader& spriteLoader;
	entt::DefaultRegistry& registry;
	// Spatial hash table with cell size equal to max(mapWidth, mapHeight)/10
	SpatialHashTable<uint32_t> defaultTable;
	// Spatial hash table with cell size equal to 2 * radius of largest hitbox; always contains all enemies and players
	SpatialHashTable<uint32_t> largeObjectsTable;
	// Cutoff size for insertion into default table; 2 * max(mapWidth, mapHeight)/10 since hitbox size is 2*radius
	float defaultTableObjectMaxSize;
};