#pragma once
#include <entt/entt.hpp>

#include <Game/Components/Components.h>

/*
This should be the only place where registry.destroy is called.
*/
class DespawnSystem {
public:
	inline DespawnSystem(entt::DefaultRegistry& registry) : registry(registry) {}
	void update(float deltaTime);

private:
	entt::DefaultRegistry& registry;
};