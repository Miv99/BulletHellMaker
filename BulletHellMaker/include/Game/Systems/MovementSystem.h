#pragma once
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>

#include <Game/Components/Components.h>
#include <Game/EntityCreationQueue.h>
#include <DataStructs/SpriteLoader.h>

/*
Handles movement and spawning of enemy/player bullets.
*/
class MovementSystem {
public:
	inline MovementSystem(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry) : queue(queue), spriteLoader(spriteLoader), registry(registry) {}
	void update(float deltaTime);

private:
	EntityCreationQueue& queue;
	SpriteLoader& spriteLoader;
	entt::DefaultRegistry& registry;
};