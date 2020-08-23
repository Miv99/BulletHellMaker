#pragma once
#include <entt/entt.hpp>

class EntityCreationQueue;

/*
System for creating a trail of shadows behind entities.
*/
class ShadowTrailSystem {
public:
	ShadowTrailSystem(EntityCreationQueue& queue, entt::DefaultRegistry& registry);

	void update(float deltaTime);

private:
	entt::DefaultRegistry& registry;
	EntityCreationQueue& queue;
};