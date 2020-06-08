#pragma once
#include <entt/entt.hpp>
#include "SpriteLoader.h"

/*
System for updating sprite animations.
Cannot be combined with RenderSystem because sprites should not undergo their animations
if the game is paused.
*/
class SpriteAnimationSystem {
public:
	SpriteAnimationSystem(SpriteLoader& spriteLoader, entt::DefaultRegistry& registry) : spriteLoader(spriteLoader), registry(registry) {}
	void update(float deltaTime);

private:
	entt::DefaultRegistry& registry;
	SpriteLoader& spriteLoader;
};