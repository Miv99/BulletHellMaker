#pragma once
#include <entt/entt.hpp>

#include <DataStructs/SpriteLoader.h>
#include <LevelPack/LevelPack.h>

class EntityCreationQueue;

/*
Manages enemies' phases and attacks
*/
class EnemySystem {
public:
	EnemySystem(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry);

	void update(float deltaTime);

private:
	EntityCreationQueue& queue;
	const LevelPack& levelPack;
	entt::DefaultRegistry& registry;
	SpriteLoader& spriteLoader;
};