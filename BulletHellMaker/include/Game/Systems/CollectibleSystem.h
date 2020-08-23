#pragma once
#include <entt/entt.hpp>

#include <DataStructs/SpatialHashTable.h>
#include <LevelPack/LevelPack.h>

class EntityCreationQueue;

/*
Manages collectibles.
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
};