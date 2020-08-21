#pragma once
#include <entt/entt.hpp>

#include <Game/Components/Components.h>
#include <LevelPack/LevelPack.h>
#include <DataStructs/SpriteLoader.h>
#include <Game/EntityCreationQueue.h>

/*
Manages enemies' phases and attacks
*/
class EnemySystem {
public:
	inline EnemySystem(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry) : queue(queue), spriteLoader(spriteLoader), levelPack(levelPack), registry(registry) {}
	void update(float deltaTime);

private:
	EntityCreationQueue& queue;
	const LevelPack& levelPack;
	entt::DefaultRegistry& registry;
	SpriteLoader& spriteLoader;
};