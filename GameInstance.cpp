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

GameInstance::GameInstance(sf::RenderWindow& window, std::string levelPackName) : window(window) {
	//TODO have gui thing like touhou + fix aspect ratio
	// Centered at negative y because SFML has (0, 0) at the top-left, and RenderSystem negates y-values so that (0, 0) in every other aspect of this game is bottom-left.
	sf::View view(sf::Vector2f(MAP_WIDTH/2.0f, -MAP_HEIGHT/2.0f), sf::Vector2f(MAP_WIDTH, MAP_HEIGHT));
	window.setView(view);

	// Read meta file
	std::ifstream metafile("Level Packs\\" + levelPackName + "\\meta.txt");
	std::unique_ptr<std::map<std::string, std::unique_ptr<std::map<std::string, std::string>>>> metadata = TextFileParser(metafile).read('=');
	std::vector<std::pair<std::string, std::string>> namePairs;
	for (auto it = metadata->at("Sprite Sheets")->begin(); it != metadata->at("Sprite Sheets")->end(); it++) {
		namePairs.push_back(std::make_pair(it->first, it->second));
	}
	spriteLoader = std::make_unique<SpriteLoader>("Level Packs\\" + levelPackName, namePairs);
	spriteLoader->preloadTextures();

	levelPack = std::make_unique<LevelPack>(levelPackName);
	queue = std::make_unique<EntityCreationQueue>(registry);

	movementSystem = std::make_unique<MovementSystem>(*queue, *spriteLoader, registry);
	renderSystem = std::make_unique<RenderSystem>(registry, window);
	collisionSystem = std::make_unique<CollisionSystem>(*queue, *spriteLoader, registry, MAP_WIDTH, MAP_HEIGHT, HitboxComponent(LOCK_ROTATION, levelPack->searchLargestHitbox(), 0, 0));
	despawnSystem = std::make_unique<DespawnSystem>(registry);
	enemySystem = std::make_unique<EnemySystem>(*queue, *spriteLoader, *levelPack, registry);
	spriteAnimationSystem = std::make_unique<SpriteAnimationSystem>(*spriteLoader, registry);
	shadowTrailSystem = std::make_unique<ShadowTrailSystem>(*queue, registry);
	playerSystem = std::make_unique<PlayerSystem>(*levelPack, *queue, *spriteLoader, registry);
	collectibleSystem = std::make_unique<CollectibleSystem>(*queue, registry);
}

void GameInstance::physicsUpdate(float deltaTime) {
	if (!paused) {
		collisionSystem->update(deltaTime);
		queue->executeAll();

		registry.get<LevelManagerTag>().update(*queue, *spriteLoader, *levelPack, registry, deltaTime);
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

	// Remove all existing entities from the registry
	registry.reset();
	reserveMemory(registry, INITIAL_ENTITY_RESERVATION);

	// Create the player
	createPlayer(levelPack->getLevel(levelIndex)->getPlayer());

	// Create the level manager
	registry.reserve<LevelManagerTag>(1);
	registry.reserve(registry.alive() + 1);
	uint32_t levelManager = registry.create();
	registry.assign<LevelManagerTag>(entt::tag_t{}, levelManager, levelPack->getLevel(levelIndex));
	
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
	registry.assign<PlayerTag>(entt::tag_t{}, player, params.getSpeed(), params.getFocusedSpeed(), params.getAttackPattern(), params.getAttackPatternLoopDelay(), params.getFocusedAttackPattern(), params.getFocusedAttackPatternLoopDelay());
	registry.assign<AnimatableSetComponent>(player, params.getAnimatableSet());
	registry.assign<HealthComponent>(player, params.getInitialHealth(), params.getMaxHealth());
	// Hitbox temporarily at 0, 0 until an Animatable is assigned to the player later
	registry.assign<HitboxComponent>(player, LOCK_ROTATION, params.getHitboxRadius(), 0, 0);
	registry.assign<PositionComponent>(player, PLAYER_SPAWN_X - params.getHitboxPosX(), PLAYER_SPAWN_Y - params.getHitboxPosY());
	registry.assign<SpriteComponent>(player, PLAYER_LAYER, 0);
}
