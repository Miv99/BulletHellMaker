#pragma once
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>

#include <DataStructs/SpriteLoader.h>

class EntityCreationQueue;

/*
Handles movement and spawning of enemy/player bullets.
*/
class MovementSystem {
public:
	MovementSystem(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry);

	void update(float deltaTime);

private:
	EntityCreationQueue& queue;
	SpriteLoader& spriteLoader;
	entt::DefaultRegistry& registry;
};