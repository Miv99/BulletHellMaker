#pragma once
#include <memory>

#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>

#include <LevelPack/AttackPattern.h>
#include <DataStructs/SpriteLoader.h>
#include <LevelPack/LevelPack.h>
#include <Game/AudioPlayer.h>

class EntityCreationQueue;

/*
Handles all things related to the player.
*/
class PlayerSystem {
public:
	PlayerSystem(LevelPack& levelPack, EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry);

	void update(float deltaTime);

	void handleEvent(sf::Event event);
	void onResume();

private:
	LevelPack& levelPack;
	EntityCreationQueue& queue;
	SpriteLoader& spriteLoader;
	entt::DefaultRegistry& registry;
};