#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <memory>
#include "MovementSystem.h"
#include "RenderSystem.h"
#include "CollisionSystem.h"
#include "EnemySystem.h"
#include "DespawnSystem.h"
#include "SpriteAnimationSystem.h"
#include "EntityCreationQueue.h"
#include "ShadowTrailSystem.h"
#include "PlayerSystem.h"
#include "CollectibleSystem.h"

class LevelPack;
class LevelManagerTag;

class GameInstance {
public:
	GameInstance(sf::RenderWindow& window, std::string levelPackName);
	void physicsUpdate(float deltaTime);
	void render(float deltaTime);

	/*
	Starts a level.
	*/
	void startLevel(int levelIndex);

	/*
	Ends a level.
	*/
	void endLevel();

	void handleEvent(sf::Event event);
	void pause();
	void resume();

private:
	std::unique_ptr<LevelPack> levelPack;
	std::unique_ptr<SpriteLoader> spriteLoader;
	std::unique_ptr<EntityCreationQueue> queue;

	entt::DefaultRegistry registry;
	sf::RenderWindow& window;

	std::unique_ptr<MovementSystem> movementSystem;
	std::unique_ptr<RenderSystem> renderSystem;
	std::unique_ptr<CollisionSystem> collisionSystem;
	std::unique_ptr<DespawnSystem> despawnSystem;
	std::unique_ptr<EnemySystem> enemySystem;
	std::unique_ptr<SpriteAnimationSystem> spriteAnimationSystem;
	std::unique_ptr<ShadowTrailSystem> shadowTrailSystem;
	std::unique_ptr<PlayerSystem> playerSystem;
	std::unique_ptr<CollectibleSystem> collectibleSystem;

	bool paused;

	// Total amount of points earned so far across all past levels.
	// Does not include points from the current level.
	int points = 0;

	void createPlayer(EditorPlayer params);
};