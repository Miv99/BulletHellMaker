#pragma once
#include <entt/entt.hpp>
#include "Components.h"
#include "EntityCreationQueue.h"

/*
Manages collectibles
*/
class CollectibleSystem {
public:
	inline CollectibleSystem(EntityCreationQueue& queue, entt::DefaultRegistry& registry) : queue(queue), registry(registry) {}
	void update(float deltaTime);

private:
	EntityCreationQueue& queue;
	entt::DefaultRegistry& registry;
};