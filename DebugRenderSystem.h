#pragma once
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <utility>
#include "Components.h"
#include "RenderSystem.h"

/*
Render system that uses no shaders and can draw outside the play area.
*/
class DebugRenderSystem : public RenderSystem {
public:
	DebugRenderSystem(entt::DefaultRegistry& registry, sf::RenderWindow& window, SpriteLoader& spriteLoader, float resolutionMultiplier);

	void update(float deltaTime) override;
	void setResolution(SpriteLoader& spriteLoader, float resolutionMultiplier) override;

private:
	sf::CircleShape circleFormat;

	using RenderSystem::loadLevelRenderSettings;
};