#pragma once
#include <entt/entt.hpp>
#include "SpatialHashTable.h"
#include "SpriteLoader.h"
#include "EntityCreationQueue.h"

class HitboxComponent;

enum BULLET_ON_COLLISION_ACTION {
	TURN_INTANGIBLE, // Hitbox is removed but sprite and children remain
	DESTROY_THIS_BULLET_AND_ATTACHED_CHILDREN, // All attached EMPs and itself are destroyed
	DESTROY_THIS_BULLET_ONLY, // Only itself is destroyed
	PIERCE_ENTITY // Do nothing on hitting an entity but bullet is unable to hit the same entity twice in some time frame (a property of the EMP)
};

class CollisionSystem {
public:
	CollisionSystem(LevelPack& levelPack, EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, float mapWidth, float mapHeight, const HitboxComponent& largestHitbox);
	void update(float deltaTime);

private:
	LevelPack& levelPack;
	EntityCreationQueue& queue;
	SpriteLoader& spriteLoader;
	entt::DefaultRegistry& registry;
	// Spatial hash table with cell size equal to max(mapWidth, mapHeight)/10
	SpatialHashTable<uint32_t> defaultTable;
	// Spatial hash table with cell size equal to 2 * radius of largest hitbox; always contains all enemies and players
	SpatialHashTable<uint32_t> largeObjectsTable;
	// Cutoff size for insertion into default table; 2 * max(mapWidth, mapHeight)/10 since hitbox size is 2*radius
	float defaultTableObjectMaxSize;
};