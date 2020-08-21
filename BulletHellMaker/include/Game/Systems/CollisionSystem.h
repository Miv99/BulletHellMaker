#pragma once
#include <entt/entt.hpp>

#include <DataStructs/SpatialHashTable.h>
#include <DataStructs/SpriteLoader.h>
#include <Game/EntityCreationQueue.h>

class HitboxComponent;

enum BULLET_ON_COLLISION_ACTION {
	DESTROY_THIS_BULLET_AND_ATTACHED_CHILDREN, // All attached EMPs and itself are destroyed
	DESTROY_THIS_BULLET_ONLY, // Only itself is destroyed
	PIERCE_ENTITY // Do nothing on hitting an entity but bullet is unable to hit the same entity twice in some time frame (a property of the EMP)
};

class CollisionSystem {
public:
	CollisionSystem(LevelPack& levelPack, EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, float mapWidth, float mapHeight);
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

	inline float distance(float x1, float y1, float x2, float y2) {
		return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
	}

	inline bool collides(const PositionComponent& p1, const HitboxComponent& h1, const PositionComponent& p2, const HitboxComponent& h2) {
		return distance(p1.getX() + h1.getX(), p1.getY() + h1.getY(), p2.getX() + h2.getX(), p2.getY() + h2.getY()) <= (h1.getRadius() + h2.getRadius());
	}
};