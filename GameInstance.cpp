#include "GameInstance.h"
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <boost/format.hpp>
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

	// GUI stuff
	// Note: "GUI region" refers to the right side of the window that doesn't contain the stuff from RenderSystem
	smoothPlayerHPBar = levelPack->getPlayer().getSmoothPlayerHPBar();

	gui = std::make_shared<tgui::Gui>(window);
	gui->setFont(tgui::Font("Level Packs\\" + levelPack->getName() + "\\" + levelPack->getFontFileName()));
	guiRegionWidth = window.getSize().x * (MAP_HEIGHT - MAP_WIDTH) / view.getSize().x;
	guiRegionHeight = window.getSize().y;
	// x-coord of left side of GUI region
	guiRegionX = window.getSize().x - guiRegionWidth;
	guiPaddingX = guiRegionWidth * 0.05f;
	guiPaddingY = guiRegionHeight * 0.03f;

	// Level name label
	levelNameLabel = tgui::Label::create();
	levelNameLabel->setTextSize(26);
	levelNameLabel->setAutoSize(true);
	levelNameLabel->setMaximumTextWidth(guiRegionWidth - guiPaddingX * 2.0f);
	levelNameLabel->setPosition({guiRegionX + guiPaddingX, guiPaddingY});
	gui->add(levelNameLabel);

	// Score label
	scoreLabel = tgui::Label::create();
	scoreLabel->setTextSize(24);
	scoreLabel->setMaximumTextWidth(0);
	scoreLabel->setPosition({ tgui::bindLeft(levelNameLabel), tgui::bindBottom(levelNameLabel) + guiPaddingY });
	gui->add(scoreLabel);

	// Power label
	powerLabel = tgui::Label::create();
	powerLabel->setTextSize(24);
	powerLabel->setMaximumTextWidth(0);
	powerLabel->setPosition({ tgui::bindLeft(scoreLabel), tgui::bindBottom(scoreLabel) + guiPaddingY });
	gui->add(powerLabel);

	if (smoothPlayerHPBar) {
		// Progress bar for player HP
		playerHPProgressBar = tgui::ProgressBar::create();
		playerHPProgressBar->setFillDirection(tgui::ProgressBar::FillDirection::LeftToRight);
		playerHPProgressBar->setMinimum(0);
		playerHPProgressBar->setMaximum(levelPack->getPlayer().getMaxHealth());
		playerHPProgressBar->setSize(guiRegionWidth - guiPaddingX * 2.0f, 22);
		playerHPProgressBar->setPosition({ guiRegionX + guiPaddingX, guiRegionHeight - playerHPProgressBar->getSize().y - guiPaddingY });
		playerHPProgressBar->getRenderer()->setBackgroundColor(tgui::Color(sf::Color(255, 170, 170, 255)));
		playerHPProgressBar->getRenderer()->setFillColor(tgui::Color::Red);
		playerHPProgressBar->getRenderer()->setTextColor(tgui::Color::White);
		playerHPProgressBar->getRenderer()->setBorderColor(tgui::Color::White);
		gui->add(playerHPProgressBar);
	} else {
		sf::Texture playerHPTexture;
		if (!playerHPTexture.loadFromFile("Level Packs\\" + levelPack->getName() + "\\" + levelPack->getPlayer().getDiscretePlayerHPSprite().getAnimatableName())) {
			//TODO: error handling
		}
		// Player HP picture can be a maximum size of 3% of guiRegionHeight
		playerHPPictureSize = std::min(guiRegionHeight * 0.03f, (guiRegionWidth - guiPaddingX * 2) / levelPack->getPlayer().getMaxHealth());

		playerHPPictures.clear();
		for (int i = 0; i < levelPack->getPlayer().getMaxHealth(); i++) {
			auto playerHPPicture = tgui::Picture::create(playerHPTexture);
			playerHPPicture->setSize(playerHPPictureSize, playerHPPictureSize);
			playerHPPictures.push_back(playerHPPicture);
		}
		playerHPPictureGrid = tgui::Grid::create();
		playerHPPictureGrid->setPosition(guiRegionX + guiPaddingX, guiRegionHeight - playerHPPictureSize - guiPaddingY);
		gui->add(playerHPPictureGrid);
		hpPicturesInGrid = 0;

		playerHPLabel = tgui::Label::create();
		playerHPLabel->setTextSize(24);
		playerHPLabel->setMaximumTextWidth(0);
		playerHPLabel->setText("Health");
		playerHPLabel->setPosition({ tgui::bindLeft(playerHPPictureGrid), tgui::bindTop(playerHPPictureGrid) - playerHPLabel->getSize().y });
		gui->add(playerHPLabel);
	}
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
	gui->draw();
}

void GameInstance::startLevel(int levelIndex) {
	paused = false;
	std::shared_ptr<Level> level = levelPack->getLevel(levelIndex);

	// Update relevant gui elements
	levelNameLabel->setText(level->getName());

	// Remove all existing entities from the registry
	registry.reset();
	reserveMemory(registry, INITIAL_ENTITY_RESERVATION);

	// Create the player
	createPlayer(levelPack->getPlayer());

	// Create the level manager
	registry.reserve<LevelManagerTag>(1);
	registry.reserve(registry.alive() + 1);
	uint32_t levelManager = registry.create();
	auto& levelManagerTag = registry.assign<LevelManagerTag>(entt::tag_t{}, levelManager, &(*levelPack), level);

	// Create the points change listener
	levelManagerTag.getPointsChangeSignal()->sink().connect<GameInstance, &GameInstance::onPointsChange>(this);

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

	// Initialize gui stuff
	onPlayerHPChange(registry.get<HealthComponent>(registry.attachee<PlayerTag>()).getHealth(), registry.get<HealthComponent>(registry.attachee<PlayerTag>()).getMaxHealth());
	onPlayerPowerLevelChange(registry.get<PlayerTag>().getCurrentPowerTierIndex(), registry.get<PlayerTag>().getPowerTierCount(), registry.get<PlayerTag>().getCurrentPower());
	onPointsChange(levelManagerTag.getPoints());

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

void GameInstance::onPlayerHPChange(int newHP, int maxHP) {
	if (smoothPlayerHPBar) {
		playerHPProgressBar->setValue(newHP);
		playerHPProgressBar->setText(std::to_string(newHP) + "/" + std::to_string(maxHP));
	} else {
		if (hpPicturesInGrid < newHP) {
			for (int i = hpPicturesInGrid; i < newHP + hpPicturesInGrid; i++) {
				playerHPPictureGrid->addWidget(playerHPPictures[i], 0, i);
				playerHPPictureGrid->setWidgetPadding(0, i, tgui::Padding(playerHPGridPadding, playerHPGridPadding, playerHPGridPadding, playerHPGridPadding));
			}
		} else {
			playerHPPictureGrid->removeAllWidgets();
			for (int i = 0; i < newHP; i++) {
				playerHPPictureGrid->addWidget(playerHPPictures[i], 0, i);
				playerHPPictureGrid->setWidgetPadding(0, i, tgui::Padding(playerHPGridPadding, playerHPGridPadding, playerHPGridPadding, playerHPGridPadding));
			}
		}
		hpPicturesInGrid = newHP;
	}
}

void GameInstance::onPointsChange(int levelPoints) {
	scoreLabel->setText((boost::format("Score\n%010d") % (points + levelPoints)).str());
}

void GameInstance::onPlayerPowerLevelChange(int powerLevelIndex, int powerLevelMaxTierCount, int powerLevel) {
	if (powerLevelIndex == powerLevelMaxTierCount - 1) {
		powerLabel->setText("Power (Lv. " + std::to_string(powerLevelMaxTierCount) + ")\nMAX");
	} else {
		powerLabel->setText("Power (Lv. " + std::to_string(powerLevelIndex + 1) + ")\n" + std::to_string(powerLevel) + "/" + std::to_string(POWER_PER_POWER_TIER));
	}
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
	auto& playerTag = registry.assign<PlayerTag>(entt::tag_t{}, player, registry, *levelPack, player, params.getSpeed(), params.getFocusedSpeed(), params.getInvulnerabilityTime(), params.getPowerTiers(), params.getHurtSound(), params.getDeathSound());
	auto& health = registry.assign<HealthComponent>(player, params.getInitialHealth(), params.getMaxHealth());
	// Hitbox temporarily at 0, 0 until an Animatable is assigned to the player later
	registry.assign<HitboxComponent>(player, LOCK_ROTATION, params.getHitboxRadius(), 0, 0);
	registry.assign<PositionComponent>(player, PLAYER_SPAWN_X - params.getHitboxPosX(), PLAYER_SPAWN_Y - params.getHitboxPosY());
	registry.assign<SpriteComponent>(player, PLAYER_LAYER, 0);

	health.getHPChangeSignal()->sink().connect<GameInstance, &GameInstance::onPlayerHPChange>(this);
	playerTag.getPowerChangeSignal()->sink().connect<GameInstance, &GameInstance::onPlayerPowerLevelChange>(this);
}