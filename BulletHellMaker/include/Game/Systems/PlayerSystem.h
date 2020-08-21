#pragma once
#include <memory>

#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>

#include <LevelPack/AttackPattern.h>
#include <Game/Components/Components.h>
#include <Game/EntityCreationQueue.h>
#include <DataStructs/SpriteLoader.h>
#include <LevelPack/LevelPack.h>
#include <Game/AudioPlayer.h>

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
};