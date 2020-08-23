#pragma once
#include <entt/entt.hpp>

/*
This should be the only place where registry.destroy() is called.
*/
class DespawnSystem {
public:
	DespawnSystem(entt::DefaultRegistry& registry);
	void update(float deltaTime);

private:
	entt::DefaultRegistry& registry;
};