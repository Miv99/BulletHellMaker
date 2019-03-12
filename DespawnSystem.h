#pragma once
#include <entt/entt.hpp>
#include "Components.h"

class DespawnSystem {
public:
	inline DespawnSystem(entt::DefaultRegistry& registry) : registry(registry) {}
	void update(float deltaTime);

private:
	entt::DefaultRegistry& registry;
};