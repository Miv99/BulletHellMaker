#include "GameInstance.h"
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include "SpriteLoader.h"
#include "TextFileParser.h"
#include "Components.h"
#include "TimeFunctionVariable.h"
#include "MovablePoint.h"
#include "LevelPack.h"
#include "Constants.h"
#include "Player.h"

GameInstance::GameInstance(sf::RenderWindow& window, std::string levelPackName) : window(window) {
	// Centered at negative y because SFML has (0, 0) at the top-left, and RenderSystem negates y-values so that (0, 0) in every other aspect of this game is bottom-left.
	sf::View view(sf::Vector2f(MAP_WIDTH/2.0f, -MAP_HEIGHT/2.0f), sf::Vector2f(MAP_WIDTH, MAP_HEIGHT));
	window.setView(view);

	audioPlayer = std::make_unique<AudioPlayer>();

	levelPack = std::make_unique<LevelPack>(*audioPlayer, levelPackName);
	queue = std::make_unique<EntityCreationQueue>(registry);

	spriteLoader = levelPack->createSpriteLoader();
	spriteLoader->preloadTextures();

	movementSystem = std::make_unique<MovementSystem>(*queue, *spriteLoader, registry);
	renderSystem = std::make_unique<RenderSystem>(registry, window);
	collisionSystem = std::make_unique<CollisionSystem>(*levelPack, *queue, *spriteLoader, registry, MAP_WIDTH, MAP_HEIGHT, HitboxComponent(LOCK_ROTATION, levelPack->searchLargestHitbox(), 0, 0));
	despawnSystem = std::make_unique<DespawnSystem>(registry);
	enemySystem = std::make_unique<EnemySystem>(*queue, *spriteLoader, *levelPack, registry);
	spriteAnimationSystem = std::make_unique<SpriteAnimationSystem>(*spriteLoader, registry);
	shadowTrailSystem = std::make_unique<ShadowTrailSystem>(*queue, registry);
	playerSystem = std::make_unique<PlayerSystem>(*levelPack, *queue, *spriteLoader, registry);
	collectibleSystem = std::make_unique<CollectibleSystem>(*queue, registry);
}

void GameInstance::physicsUpdate(float deltaTime) {
	if (!paused) {
		audioPlayer->update(deltaTime);

		collisionSystem->update(deltaTime);
		queue->executeAll();

		registry.get<LevelManagerTag>().update(*queue, *spriteLoader, registry, deltaTime);
		queue->executeAll();

		shadowTrailSystem->update(deltaTime);
		queue->executeAll();

		movementSystem->update(deltaTime);
		queue->executeAll();

		collectibleSystem->update(deltaTime);
		queue->executeAll();

		playerSystem->update(deltaTime);
		queue->executeAll();

		enemySystem->update(deltaTime);
		queue->executeAll();

		despawnSystem->update(deltaTime);
		queue->executeAll();
	}
}

void GameInstance::render(float deltaTime) {
	if (!paused) {
		spriteAnimationSystem->update(deltaTime);
	}
	renderSystem->update(deltaTime);
}

void GameInstance::startLevel(int levelIndex) {
	paused = false;
	std::shared_ptr<Level> level = levelPack->getLevel(levelIndex);

	// Remove all existing entities from the registry
	registry.reset();
	reserveMemory(registry, INITIAL_ENTITY_RESERVATION);

	// Create the player
	createPlayer(levelPack->getPlayer());

	// Create the level manager
	registry.reserve<LevelManagerTag>(1);
	registry.reserve(registry.alive() + 1);
	uint32_t levelManager = registry.create();
	registry.assign<LevelManagerTag>(entt::tag_t{}, levelManager, &(*levelPack), level);
	
	// Play level music
	levelPack->playMusic(level->getMusicSettings());

	// Set the background
	std::string backgroundFileName = level->getBackgroundFileName();
	sf::Texture background;
	if (!background.loadFromFile("Level Packs\\" + levelPack->getName() + "\\Backgrounds\\" + backgroundFileName)) {
		//TODO: load a default background
	}
	background.setRepeated(true);
	background.setSmooth(true);
	renderSystem->setBackground(std::move(background));
	renderSystem->setBackgroundScrollSpeedX(level->getBackgroundScrollSpeedX());
	renderSystem->setBackgroundScrollSpeedY(level->getBackgroundScrollSpeedY());

	resume();
}

void GameInstance::endLevel() {
	//TODO

	// Add points from the level that is ending
	points += registry.get<LevelManagerTag>().getPoints();
}

void GameInstance::handleEvent(sf::Event event) {
	playerSystem->handleEvent(event);
}

void GameInstance::pause() {
}

void GameInstance::resume() {
	playerSystem->onResume();
}

void GameInstance::createPlayer(EditorPlayer params) {
	registry.reserve(1);
	registry.reserve<PlayerTag>(1);
	registry.reserve<AnimatableSetComponent>(1);
	registry.reserve<HealthComponent>(1);
	registry.reserve<HitboxComponent>(1);
	registry.reserve<PositionComponent>(1);
	registry.reserve<SpriteComponent>(1);

	auto player = registry.create();
	registry.assign<AnimatableSetComponent>(player);
	registry.assign<PlayerTag>(entt::tag_t{}, player, registry, *levelPack, player, params.getSpeed(), params.getFocusedSpeed(), params.getInvulnerabilityTime(), params.getPowerTiers());
	registry.assign<HealthComponent>(player, params.getInitialHealth(), params.getMaxHealth());
	// Hitbox temporarily at 0, 0 until an Animatable is assigned to the player later
	registry.assign<HitboxComponent>(player, LOCK_ROTATION, params.getHitboxRadius(), 0, 0);
	registry.assign<PositionComponent>(player, PLAYER_SPAWN_X - params.getHitboxPosX(), PLAYER_SPAWN_Y - params.getHitboxPosY());
	registry.assign<SpriteComponent>(player, PLAYER_LAYER, 0);
}
