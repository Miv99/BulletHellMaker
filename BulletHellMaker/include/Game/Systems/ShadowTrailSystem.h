#pragma once
#include <entt/entt.hpp>

#include <Game/EntityCreationQueue.h>

/*
System for creating a trail of shadows behind entities.
*/
class ShadowTrailSystem {
public:
	ShadowTrailSystem(EntityCreationQueue& queue, entt::DefaultRegistry& registry) : queue(queue), registry(registry) {}
	void update(float deltaTime);

private:
	entt::DefaultRegistry& registry;
	EntityCreationQueue& queue;
};