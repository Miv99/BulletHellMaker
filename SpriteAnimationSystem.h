#pragma once
#include <entt/entt.hpp>

/*
System for updating sprite animations.
Cannot be combined with RenderSystem because sprites should not undergo their animations
if the game is paused.
*/
class SpriteAnimationSystem {
public:
	SpriteAnimationSystem(entt::DefaultRegistry& registry) : registry(registry) {}
	void update(float deltaTime);

private:
	entt::DefaultRegistry& registry;
};