#pragma once
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include "AttackPattern.h"
#include "Components.h"
#include "EntityCreationQueue.h"
#include "SpriteLoader.h"
#include "LevelPack.h"

/*
Handles all things related to the player.
*/
class PlayerSystem {
public:
	inline PlayerSystem(LevelPack& levelPack, EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry) : levelPack(levelPack), queue(queue), spriteLoader(spriteLoader), registry(registry) {}
	void update(float deltaTime);

	void handleEvent(sf::Event event);
	void onResume();

private:
	LevelPack& levelPack;
	EntityCreationQueue& queue;
	SpriteLoader& spriteLoader;
	entt::DefaultRegistry& registry;

	int verticalInput = 0;
	int horizontalInput = 0;

	bool focused = false;

	bool attacking = false;
	float timeSinceNewAttackPattern = 0;
	int currentAttackIndex = -1;
};