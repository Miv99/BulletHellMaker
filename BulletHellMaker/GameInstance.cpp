#include "GameInstance.h"
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <algorithm>
#include <boost/format.hpp>
#include "SpriteLoader.h"
#include "TextFileParser.h"
#include "Components.h"
#include "TimeFunctionVariable.h"
#include "MovablePoint.h"
#include "LevelPack.h"
#include "Constants.h"
#include "Player.h"
#include "Level.h"
#include "Enemy.h"
#include "EnemyPhaseStartCondition.h"

#include <iostream>

void GameInstance::updateWindowView(int windowWidth, int windowHeight) {
	sf::Vector2u resolution = renderSystem->getResolution();
	sf::View view(sf::FloatRect(0, 0, resolution.x, resolution.y));

	// Compares the aspect ratio of the window to the aspect ratio of the view,
	// and sets the view's viewport accordingly in order to archieve a letterbox effect.
	// A new view (with a new viewport set) is returned.
	float windowRatio = windowWidth / (float)windowHeight;
	float playAreaViewRatio = view.getSize().x / (float)view.getSize().y;
	// size of play area + gui region
	float playAreaSizeX = 1;
	float playAreaSizeY = 1;
	float guiAreaSizeX = 1;
	float posX = 0;
	float posY = 0;

	bool horizontalSpacing = true;
	if (windowRatio < playAreaViewRatio)
		horizontalSpacing = false;

	// If horizontalSpacing is true, the black bars will appear on the left and right side.
	// Otherwise, the black bars will appear on the top and bottom.
	if (horizontalSpacing) {
		playAreaSizeX = playAreaViewRatio / windowRatio;
		guiAreaSizeX = std::min(1 - playAreaSizeX, std::max(playAreaSizeX * 0.5f, (float)scoreLabel->getSize().x/windowWidth));
		posX = (1 - playAreaSizeX - guiAreaSizeX) / 2.f;
	} else {
		playAreaSizeY = windowRatio / playAreaViewRatio;
		posY = (1 - playAreaSizeY) / 2.f;
	}

	view.setViewport(sf::FloatRect(posX, posY, playAreaSizeX, playAreaSizeY));
	// Make sure nothing in the gui autoscales
	gui->setView(sf::View(sf::Vector2f(windowWidth/2.0f, windowHeight/2.0f), sf::Vector2f(windowWidth, windowHeight)));

	// Recalculate widths/positions of gui elements
	//TODO: redo this entire part; what do sizeX and sizeY really represent????
	guiRegionX = (playAreaSizeX + posX) * windowWidth;
	guiRegionYLow = posY * windowHeight;
	guiRegionYHigh = (playAreaSizeY + posY) * windowHeight;
	guiRegionWidth = guiAreaSizeX * windowWidth;
	guiRegionHeight = guiRegionYHigh - guiRegionYLow;
	playAreaX = posX * windowWidth;
	float playAreaWidth = playAreaSizeX * windowWidth;
	// Since everything in the gui is bound to levelNameLabel's x position, only levelNameLabel's position needs to be updated 
	levelNameLabel->setPosition({ guiRegionX + guiPaddingX, guiPaddingY });

	levelNameLabel->setMaximumTextWidth(guiRegionWidth - guiPaddingX * 2.0f);

	bombPictureSize = std::max(bombPictureSizeMin, std::min(bombPictureSizeMax, (guiRegionWidth - guiPaddingX * 2) / playerInfo->getMaxBombs()));
	bombPictureDisplayMax = (int)(guiRegionWidth - 2 * guiPaddingX) / (bombPictureSize + guiPaddingX);
	for (int i = 0; i < bombPictures.size(); i++) {
		bombPictures[i]->setSize(bombPictureSize, bombPictureSize);
	}
	if (registry.valid(registry.attachee<PlayerTag>())) {
		onPlayerBombCountChange(registry.get<PlayerTag>().getBombCount());
	}

	bombPictureGrid->setPosition({ tgui::bindLeft(levelNameLabel), guiRegionHeight - bombPictureSize - guiPaddingY });

	if (playerHPProgressBar) {
		playerHPProgressBar->setSize(guiRegionWidth - guiPaddingX * 2.0f, 22);
	} else {
		playerHPPictureSize = std::max(playerHPPictureSizeMin, std::min(playerHPPictureSizeMax, (guiRegionWidth - guiPaddingX * 2) / playerInfo->getMaxHealth()));
		playerHPPictureDisplayMax = (int)(guiRegionWidth - 2 * guiPaddingX) / (playerHPPictureSize + guiPaddingX);
		for (int i = 0; i < playerHPPictures.size(); i++) {
			playerHPPictures[i]->setSize(playerHPPictureSize, playerHPPictureSize);
		}
		playerHPPictureGrid->setPosition({ tgui::bindLeft(bombPictureGrid), tgui::bindTop(bombLabel) - guiPaddingY - playerHPPictureSize });
		if (registry.valid(registry.attachee<PlayerTag>())) {
			onPlayerHPChange(registry.get<HealthComponent>(registry.attachee<PlayerTag>()).getHealth(), registry.get<HealthComponent>(registry.attachee<PlayerTag>()).getMaxHealth());
		}
	}

	bossLabel->setPosition(playAreaX, guiRegionYLow);

	bossPhaseHealthBar->setSize(playAreaWidth, bossPhaseHealthBarHeight);
	bossPhaseHealthBar->setPosition(playAreaX, guiRegionYLow);

	if (bossPhaseTimeLeft->isVisible() && registry.valid(registry.attachee<PlayerTag>())) {
		uint32_t player = registry.attachee<PlayerTag>();
		auto& playerHitbox = registry.get<HitboxComponent>(player);
		auto& pos = registry.get<PositionComponent>(player);
		if (pos.getX() + playerHitbox.getX() - playerHitbox.getRadius() < MAP_WIDTH / 2.0f) {
			bossPhaseTimeLeft->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Right);
			bossPhaseTimeLeft->setPosition(guiRegionX - bossPhaseTimeLeft->getSize().x, guiRegionYHigh - bossPhaseTimeLeft->getSize().y);
		} else {
			bossPhaseTimeLeft->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Left);
			bossPhaseTimeLeft->setPosition(playAreaX, guiRegionYHigh - bossPhaseTimeLeft->getSize().y);
		}
	}

	calculateDialogueBoxWidgetsSizes();

	window->setView(view);
}

void GameInstance::calculateDialogueBoxWidgetsSizes() {
	// Only update widget sizes if there is a dialogue event currently in progress
	if (dialogueBoxTextsQueueIndex != -1) {
		if (dialogueBoxPortraitPicture->isVisible()) {
			dialogueBoxPortraitPicture->setSize(dialogueBoxPortraitPicture->getRenderer()->getTexture().getImageSize());
			dialogueBoxPicture->setSize(guiRegionX - GUI_PADDING_X - (dialogueBoxPortraitPicture->getPosition().x - dialogueBoxPortraitPicture->getSize().x), tgui::bindHeight(dialogueBoxPortraitPicture));
			dialogueBoxLabel->setMaximumTextWidth(dialogueBoxPicture->getSize().x - DIALOGUE_BOX_PADDING * 2);
		} else {
			dialogueBoxPicture->setSize(guiRegionX - GUI_PADDING_X * 2, tgui::bindHeight(dialogueBoxPortraitPicture));
			dialogueBoxLabel->setMaximumTextWidth(dialogueBoxPicture->getSize().x - DIALOGUE_BOX_PADDING * 2);
		}
	}
}

GameInstance::GameInstance(std::string levelPackName) {
	audioPlayer = std::make_unique<AudioPlayer>();
	levelPack = std::make_unique<LevelPack>(*audioPlayer, levelPackName);
	playerInfo = levelPack->getGameplayPlayer();

	//TODO: these numbers should come from settings
	window = std::make_unique<sf::RenderWindow>(sf::VideoMode(1600, 900), "Bullet Hell Maker");
	window->setKeyRepeatEnabled(false);

	// Centered at negative y because SFML has (0, 0) at the top-left, and RenderSystem negates y-values so that (0, 0) in every other aspect of this game is bottom-left.
	sf::View view(sf::FloatRect(0, -MAP_HEIGHT, MAP_WIDTH, MAP_HEIGHT));
	window->setView(view);

	// Create these first because updateWindowView uses them
	gui = std::make_shared<tgui::Gui>(*window);
	levelNameLabel = tgui::Label::create();
	bossPhaseTimeLeft = tgui::Label::create();
	bossPhaseHealthBar = tgui::ProgressBar::create();
	bombPictureGrid = tgui::Grid::create();
	bombLabel = tgui::Label::create();
	bossLabel = tgui::Label::create();
	sf::Texture bombTexture;
	if (!bombTexture.loadFromFile("Level Packs\\" + levelPack->getName() + "\\" + playerInfo->getBombSprite().getAnimatableName())) {
		//TODO: error handling
	}
	for (int i = 0; i < playerInfo->getMaxBombs(); i++) {
		auto bombPicture = tgui::Picture::create(bombTexture);
		bombPicture->setSize(bombPictureSize, bombPictureSize);
		bombPictures.push_back(bombPicture);
	}
	if (!smoothPlayerHPBar) {
		playerHPPictureGrid = tgui::Grid::create();

		sf::Texture playerHPTexture;
		if (!playerHPTexture.loadFromFile("Level Packs\\" + levelPack->getName() + "\\" + playerInfo->getDiscretePlayerHPSprite().getAnimatableName())) {
			//TODO: error handling
		}

		for (int i = 0; i < playerInfo->getMaxHealth(); i++) {
			auto playerHPPicture = tgui::Picture::create(playerHPTexture);
			playerHPPicture->setSize(playerHPPictureSize, playerHPPictureSize);
			playerHPPictures.push_back(playerHPPicture);
		}
	}
	scoreLabel = tgui::Label::create();
	scoreLabel->setTextSize(24);
	scoreLabel->setMaximumTextWidth(0);
	scoreLabel->setText((boost::format("Score\n%010d") % 0).str());

	windowHeight = window->getSize().y;

	queue = std::make_unique<EntityCreationQueue>(registry);

	spriteLoader = levelPack->createSpriteLoader();
	spriteLoader->preloadTextures();

	movementSystem = std::make_unique<MovementSystem>(*queue, *spriteLoader, registry);
	//TODO: these numbers should come from settings
	renderSystem = std::make_unique<RenderSystem>(registry, *window, *spriteLoader, 1.0f);
	collisionSystem = std::make_unique<CollisionSystem>(*levelPack, *queue, *spriteLoader, registry, MAP_WIDTH, MAP_HEIGHT);
	despawnSystem = std::make_unique<DespawnSystem>(registry);
	enemySystem = std::make_unique<EnemySystem>(*queue, *spriteLoader, *levelPack, registry);
	spriteAnimationSystem = std::make_unique<SpriteAnimationSystem>(*spriteLoader, registry);
	shadowTrailSystem = std::make_unique<ShadowTrailSystem>(*queue, registry);
	playerSystem = std::make_unique<PlayerSystem>(*levelPack, *queue, *spriteLoader, registry);
	collectibleSystem = std::make_unique<CollectibleSystem>(*queue, registry, *levelPack, MAP_WIDTH, MAP_HEIGHT);

	// GUI stuff

	// Note: "GUI region" refers to the right side of the window that doesn't contain the stuff from RenderSystem
	smoothPlayerHPBar = playerInfo->getSmoothPlayerHPBar();

	gui->setFont(tgui::Font("Level Packs\\" + levelPack->getName() + "\\" + levelPack->getFontFileName()));

	// Level name label
	levelNameLabel->setTextSize(26);
	levelNameLabel->setAutoSize(true);
	levelNameLabel->setMaximumTextWidth(guiRegionWidth - guiPaddingX * 2.0f);
	levelNameLabel->setPosition({guiRegionX + guiPaddingX, guiPaddingY});
	gui->add(levelNameLabel);

	// Score label
	scoreLabel->setPosition({ tgui::bindLeft(levelNameLabel), tgui::bindBottom(levelNameLabel) + guiPaddingY });
	gui->add(scoreLabel);

	// Power label
	powerLabel = tgui::Label::create();
	powerLabel->setTextSize(24);
	powerLabel->setMaximumTextWidth(0);
	powerLabel->setPosition({ tgui::bindLeft(levelNameLabel), tgui::bindBottom(scoreLabel) + guiPaddingY });
	gui->add(powerLabel);

	// Bomb grid
	bombPictureGrid->setPosition({ tgui::bindLeft(levelNameLabel), guiRegionHeight - bombPictureSize - guiPaddingY });
	gui->add(bombPictureGrid);
	bombPicturesInGrid = 0;

	// Bomb label
	bombLabel->setTextSize(24);
	bombLabel->setMaximumTextWidth(0);
	bombLabel->setText("Bombs");
	bombLabel->setPosition({ tgui::bindLeft(bombPictureGrid), tgui::bindTop(bombPictureGrid) - bombLabel->getSize().y });
	gui->add(bombLabel);

	bombCountLabel = tgui::Label::create();
	bombCountLabel->setTextSize(24);
	bombCountLabel->setMaximumTextWidth(0);

	if (smoothPlayerHPBar) {
		// Progress bar for player HP
		playerHPProgressBar = tgui::ProgressBar::create();
		playerHPProgressBar->setFillDirection(tgui::ProgressBar::FillDirection::LeftToRight);
		playerHPProgressBar->setMinimum(0);
		playerHPProgressBar->setMaximum(playerInfo->getMaxHealth());
		playerHPProgressBar->setSize(guiRegionWidth - guiPaddingX * 2.0f, 22);
		playerHPProgressBar->setPosition({ tgui::bindLeft(levelNameLabel), guiRegionHeight - playerHPProgressBar->getSize().y - guiPaddingY });
		playerHPProgressBar->getRenderer()->setBackgroundColor(tgui::Color(sf::Color(255, 170, 170, 255)));
		playerHPProgressBar->getRenderer()->setFillColor(tgui::Color::Red);
		playerHPProgressBar->getRenderer()->setTextColor(tgui::Color::White);
		playerHPProgressBar->getRenderer()->setBorderColor(tgui::Color::White);
		gui->add(playerHPProgressBar);
	} else {
		playerHPPictureGrid->setPosition({ tgui::bindLeft(bombPictureGrid), tgui::bindTop(bombLabel) - guiPaddingY - playerHPPictureSize });
		gui->add(playerHPPictureGrid);
		hpPicturesInGrid = 0;

		playerHPLabel = tgui::Label::create();
		playerHPLabel->setTextSize(24);
		playerHPLabel->setMaximumTextWidth(0);
		playerHPLabel->setText("Health");
		playerHPLabel->setPosition({ tgui::bindLeft(playerHPPictureGrid), tgui::bindTop(playerHPPictureGrid) - playerHPLabel->getSize().y });
		gui->add(playerHPLabel);

		playerDiscreteHPCountLabel = tgui::Label::create();
		playerDiscreteHPCountLabel->setTextSize(24);
		playerDiscreteHPCountLabel->setMaximumTextWidth(0);
	}

	// Boss stuff
	bossLabel->setTextSize(26);
	bossLabel->setMaximumTextWidth(window->getSize().x - guiRegionWidth);
	bossLabel->setVisible(false);
	gui->add(bossLabel);

	bossPhaseTimeLeft->setTextSize(26);
	bossPhaseTimeLeft->setMaximumTextWidth(0);
	bossPhaseTimeLeft->setVisible(false);
	gui->add(bossPhaseTimeLeft);

	bossPhaseHealthBar->setFillDirection(tgui::ProgressBar::FillDirection::LeftToRight);
	bossPhaseHealthBar->getRenderer()->setBackgroundColor(tgui::Color::Transparent);
	bossPhaseHealthBar->getRenderer()->setFillColor(tgui::Color::Red);
	bossPhaseHealthBar->getRenderer()->setTextColor(tgui::Color::White);
	bossPhaseHealthBar->getRenderer()->setBorderColor(tgui::Color::Transparent);
	bossPhaseHealthBar->setVisible(false);
	gui->add(bossPhaseHealthBar);

	// Dialogue box
	dialogueBoxPicture = tgui::Picture::create();
	dialogueBoxPortraitPicture = tgui::Picture::create();
	dialogueBoxLabel = TimedLabel::create();

	calculateDialogueBoxWidgetsSizes();

	dialogueBoxPicture->setPosition(tgui::bindRight(dialogueBoxPortraitPicture), tgui::bindTop(dialogueBoxPortraitPicture));
	dialogueBoxLabel->setPosition(tgui::bindLeft(dialogueBoxPicture) + DIALOGUE_BOX_PADDING, tgui::bindTop(dialogueBoxPicture) + DIALOGUE_BOX_PADDING);

	dialogueBoxPicture->setVisible(false);
	dialogueBoxPortraitPicture->setVisible(false);
	dialogueBoxLabel->setVisible(false);

	updateWindowView(window->getSize().x, window->getSize().y);
}

void GameInstance::start() {
	sf::Clock deltaClock;

	// Game loop
	while (window->isOpen() && !gameInstanceCloseQueued) {
		// While behind in render updates, do physics updates
		float timeSinceLastRender = 0;
		while (timeSinceLastRender < RENDER_INTERVAL) {
			sf::Event event;
			while (window->pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					window->close();
				} else if (event.type == sf::Event::Resized) {
					updateWindowView(event.size.width, event.size.height);
				} else {
					handleEvent(event);
				}
			}

			if (paused) {
				break;
			}

			float dt = std::min(MAX_PHYSICS_DELTA_TIME, deltaClock.restart().asSeconds());
			//float dt = 1 / 120.0f;
			//std::cout << registry.alive() << std::endl;
			physicsUpdate(dt);

			timeSinceLastRender += dt;
		}

		window->clear();
		render(timeSinceLastRender);
		window->display();
	}
}

void GameInstance::close() {
	gameInstanceCloseQueued = true;
}

void GameInstance::physicsUpdate(float deltaTime) {
	if (!paused) {
		audioPlayer->update(deltaTime);

		collisionSystem->update(deltaTime);
		queue->executeAll();

		registry.get<LevelManagerTag>().update(*queue, *spriteLoader, registry, deltaTime);
		queue->executeAll();

		despawnSystem->update(deltaTime);
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
	}
}

void GameInstance::render(float deltaTime) {
	if (!paused) {
		spriteAnimationSystem->update(deltaTime);
	}
	renderSystem->update(deltaTime);

	// Update bomb opacity depending on time left until cooldown is over
	auto& playerTag = registry.get<PlayerTag>();
	float opacity = std::min(playerTag.getTimeSinceLastBombActivation()/playerTag.getBombCooldown(), 1.0f);
	bombPictureGrid->setInheritedOpacity(opacity);

	if (bossPhaseHealthBar->isVisible()) {
		bossPhaseHealthBar->setValue(registry.get<HealthComponent>(currentBoss).getHealth());
	}
	if (bossPhaseTimeLeft->isVisible()) {
		bossPhaseTimeLeft->setText((boost::format("%.2f") % (bossNextPhaseStartTime - registry.get<EnemyComponent>(currentBoss).getTimeSinceLastPhase())).str());

		// Move timer based on player position so that the user's vision isn't obstructed
		uint32_t player = registry.attachee<PlayerTag>();
		auto& playerHitbox = registry.get<HitboxComponent>(player);
		auto& pos = registry.get<PositionComponent>(player);
		if (pos.getY() + playerHitbox.getY() - playerHitbox.getRadius() < MAP_HEIGHT / 4.0f) {
			if (pos.getX() + playerHitbox.getX() - playerHitbox.getRadius() < MAP_WIDTH / 3.0f) {
				bossPhaseTimeLeft->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Right);
				bossPhaseTimeLeft->setPosition(guiRegionX - bossPhaseTimeLeft->getSize().x, guiRegionYHigh - bossPhaseTimeLeft->getSize().y);
			} else if (pos.getX() + playerHitbox.getX() + playerHitbox.getRadius() > MAP_WIDTH * 2 / 3.0f) {
				bossPhaseTimeLeft->setHorizontalAlignment(tgui::Label::HorizontalAlignment::Left);
				bossPhaseTimeLeft->setPosition(playAreaX, guiRegionYHigh - bossPhaseTimeLeft->getSize().y);
			}
		}
	}

	gui->draw();
}

void GameInstance::loadLevel(int levelIndex) {
	currentLevel = levelPack->getGameplayLevel(levelIndex);

	// Load bloom settings
	renderSystem->loadLevelRenderSettings(currentLevel);

	// Update relevant gui elements
	levelNameLabel->setText(currentLevel->getName());

	// Remove all existing entities from the registry
	registry.reset();
	reserveMemory(registry, INITIAL_ENTITY_RESERVATION);

	// Create the level manager
	registry.reserve<LevelManagerTag>(1);
	registry.reserve(registry.alive() + 1);
	uint32_t levelManager = registry.create();
	auto& levelManagerTag = registry.assign<LevelManagerTag>(entt::tag_t{}, levelManager, &(*levelPack), currentLevel);

	// Create the player
	createPlayer(*playerInfo);

	// Create the points change listener
	levelManagerTag.getPointsChangeSignal()->sink().connect<GameInstance, &GameInstance::onPointsChange>(this);

	// Play level music
	currentLevelMusic = levelPack->playMusic(currentLevel->getMusicSettings());

	// Set the background
	std::string backgroundFileName = currentLevel->getBackgroundFileName();
	sf::Texture background;
	if (!background.loadFromFile("Level Packs\\" + levelPack->getName() + "\\Backgrounds\\" + backgroundFileName)) {
		//TODO: load a default background
	}
	background.setRepeated(true);
	background.setSmooth(true);
	renderSystem->setBackground(std::move(background));
	renderSystem->setBackgroundScrollSpeedX(currentLevel->getBackgroundScrollSpeedX());
	renderSystem->setBackgroundScrollSpeedY(currentLevel->getBackgroundScrollSpeedY());
	renderSystem->setBackgroundTextureWidth(currentLevel->getBackgroundTextureWidth());
	renderSystem->setBackgroundTextureHeight(currentLevel->getBackgroundTextureHeight());

	// Initialize gui stuff
	onPlayerHPChange(registry.get<HealthComponent>(registry.attachee<PlayerTag>()).getHealth(), registry.get<HealthComponent>(registry.attachee<PlayerTag>()).getMaxHealth());
	onPlayerPowerLevelChange(registry.get<PlayerTag>().getCurrentPowerTierIndex(), registry.get<PlayerTag>().getPowerTierCount(), registry.get<PlayerTag>().getCurrentPower());
	onPointsChange(levelManagerTag.getPoints());
	onPlayerBombCountChange(registry.get<PlayerTag>().getBombCount());
}

void GameInstance::endLevel() {
	//TODO

	// Add points from the level that is ending
	points += registry.get<LevelManagerTag>().getPoints();
}

void GameInstance::handleEvent(sf::Event event) {
	gui->handleEvent(event);
	playerSystem->handleEvent(event);
}

void GameInstance::pause() {
	paused = true;
}

void GameInstance::resume() {
	paused = false;
	playerSystem->onResume();
}

void GameInstance::showDialogue(ShowDialogueLevelEvent* dialogueEvent) {
	std::pair<std::string, sf::IntRect> dialogueBoxIdentifier = std::make_pair(dialogueEvent->getDialogueBoxTextureFileName(), dialogueEvent->getTextureMiddlePart());

	// Load dialogue box texture
	std::shared_ptr<sf::Texture> dialogueBoxTexture;
	if (dialogueBoxTextures.contains(dialogueBoxIdentifier)) {
		dialogueBoxTexture = dialogueBoxTextures.get(dialogueBoxIdentifier);
	} else {
		dialogueBoxTexture = std::make_shared<sf::Texture>();
		if (!dialogueBoxTexture->loadFromFile(dialogueBoxIdentifier.first, dialogueBoxIdentifier.second)) {
			//TODO: load some default dialogue box texture
		}
		dialogueBoxTextures.insert(dialogueBoxIdentifier, dialogueBoxTexture);
	}
	dialogueBoxPicture->getRenderer()->setTexture(*dialogueBoxTexture);

	// Load portrait texture
	if (dialogueEvent->getDialogueBoxPortraitFileName() == "") {
		dialogueBoxPortraitPicture->setVisible(false);
		dialogueBoxPortraitPicture->setSize(0, 0);
	} else {
		std::shared_ptr<sf::Texture> dialogueBoxPortraitTexture;
		if (dialogueBoxPortraitTextures.contains(dialogueEvent->getDialogueBoxPortraitFileName())) {
			dialogueBoxPortraitTexture = dialogueBoxPortraitTextures.get(dialogueEvent->getDialogueBoxPortraitFileName());
		} else {
			dialogueBoxPortraitTexture = std::make_shared<sf::Texture>();
			if (!dialogueBoxPortraitTexture->loadFromFile(dialogueEvent->getDialogueBoxPortraitFileName())) {
				//TODO: load some default dialogue box portrait texture
			}
			dialogueBoxPortraitTextures.insert(dialogueEvent->getDialogueBoxPortraitFileName(), dialogueBoxPortraitTexture);
		}
		dialogueBoxPortraitPicture->getRenderer()->setTexture(*dialogueBoxPortraitTexture);
	}

	// Set positions
	if (dialogueEvent->getDialogueBoxPosition() == ShowDialogueLevelEvent::PositionOnScreen::BOTTOM) {
		dialogueBoxPortraitPicture->setPosition(GUI_PADDING_X, windowHeight - GUI_PADDING_Y - dialogueBoxPortraitPicture->getSize().y);
	} else if (dialogueEvent->getDialogueBoxPosition() == ShowDialogueLevelEvent::PositionOnScreen::TOP) {
		dialogueBoxPortraitPicture->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	} else {
		// Missed a case
		assert(false);
	}
	dialogueBoxPicture->setPosition(tgui::bindRight(dialogueBoxPortraitPicture), tgui::bindTop(dialogueBoxPortraitPicture));
	dialogueBoxLabel->setPosition(tgui::bindLeft(dialogueBoxPicture) + DIALOGUE_BOX_PADDING, tgui::bindTop(dialogueBoxPicture) + DIALOGUE_BOX_PADDING);

	// Load texts
	dialogeBoxTextsQueue = dialogueEvent->getText();
	dialogueBoxTextsQueueIndex = 0;

	dialogueBoxPicture->showWithEffect(dialogueEvent->getDialogueBoxShowAnimationType(), sf::seconds(dialogueEvent->getDialogueBoxShowAnimationTime()));

	calculateDialogueBoxWidgetsSizes();
}

void GameInstance::onPlayerHPChange(int newHP, int maxHP) {
	if (smoothPlayerHPBar) {
		playerHPProgressBar->setValue(newHP);
		playerHPProgressBar->setText(std::to_string(newHP) + "/" + std::to_string(maxHP));
	} else {
		playerHPPictureGrid->removeAllWidgets();
		if (newHP >= playerHPPictureDisplayMax) {
			playerHPPictureGrid->addWidget(playerHPPictures[0], 0, 0);
			playerDiscreteHPCountLabel->setText("x" + std::to_string(newHP));
			playerHPPictureGrid->addWidget(playerDiscreteHPCountLabel, 0, 1);
		} else {
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
		powerLabel->setText("Power (Lv. " + std::to_string(powerLevelIndex + 1) + ")\n" + std::to_string(powerLevel) + "/" + std::to_string(registry.get<PlayerTag>().getPowerToNextTier()));
	}
}

void GameInstance::onPlayerBombCountChange(int newBombCount) {
	if (bombPicturesInGrid < newBombCount) {
		for (int i = bombPicturesInGrid; i < newBombCount; i++) {
			bombPictureGrid->addWidget(bombPictures[i], 0, i);
			bombPictureGrid->setWidgetPadding(0, i, tgui::Padding(bombGridPadding, bombGridPadding, bombGridPadding, bombGridPadding));
		}
	} else {
		bombPictureGrid->removeAllWidgets();
		if (newBombCount >= bombPictureDisplayMax) {
			bombPictureGrid->addWidget(bombPictures[0], 0, 0);
			bombCountLabel->setText("x" + std::to_string(newBombCount));
			bombPictureGrid->addWidget(playerDiscreteHPCountLabel, 0, 1);
		} else {
			for (int i = 0; i < newBombCount; i++) {
				bombPictureGrid->addWidget(bombPictures[i], 0, i);
				bombPictureGrid->setWidgetPadding(0, i, tgui::Padding(bombGridPadding, bombGridPadding, bombGridPadding, bombGridPadding));
			}
		}
	}
	bombPicturesInGrid = newBombCount;
}

void GameInstance::onEnemySpawn(uint32_t enemy) {
	auto& enemyComponent = registry.get<EnemyComponent>(enemy);
	const std::shared_ptr<EditorEnemy> data = enemyComponent.getEnemyData();
	if (!data->getIsBoss()) {
		return;
	}
	
	bossLabel->setVisible(true);
	bossLabel->setText(data->getName());

	currentBoss = enemy;
	enemyComponent.getEnemyPhaseChangeSignal()->sink().connect<GameInstance, &GameInstance::onBossPhaseChange>(this);
	registry.get<DespawnComponent>(enemy).getDespawnSignal()->sink().connect<GameInstance, &GameInstance::onBossDespawn>(this);
}

void GameInstance::onBossPhaseChange(uint32_t boss, std::shared_ptr<EditorEnemyPhase> newPhase, std::shared_ptr<EnemyPhaseStartCondition> previousPhaseStartCondition, std::shared_ptr<EnemyPhaseStartCondition> nextPhaseStartCondition) {
	auto& enemyComponent = registry.get<EnemyComponent>(boss);
	const std::shared_ptr<EditorEnemy> data = enemyComponent.getEnemyData();
	if (!nextPhaseStartCondition || std::dynamic_pointer_cast<HPBasedEnemyPhaseStartCondition>(nextPhaseStartCondition)) {
		// Show phase HP bar
		bossPhaseTimeLeft->setVisible(false);
		bossPhaseHealthBar->setVisible(true);
		// No need to push boss name label down to make room for HP bar because the label already has enough space on top for the bar

		if (!nextPhaseStartCondition) {
			bossPhaseHealthBar->setMinimum(0);
		} else {
			bossPhaseHealthBar->setMinimum(registry.get<HealthComponent>(boss).getMaxHealth() * std::dynamic_pointer_cast<HPBasedEnemyPhaseStartCondition>(nextPhaseStartCondition)->getRatio());
		}
		if (std::dynamic_pointer_cast<HPBasedEnemyPhaseStartCondition>(previousPhaseStartCondition)) {
			bossPhaseHealthBar->setMaximum(registry.get<HealthComponent>(boss).getMaxHealth() * std::dynamic_pointer_cast<HPBasedEnemyPhaseStartCondition>(previousPhaseStartCondition)->getRatio());
		} else {
			// If previous phase start condition was not HP based, set the maximum of the progress bar to be whatever the last known health is
			bossPhaseHealthBar->setMaximum(registry.get<HealthComponent>(boss).getHealth());
		}
	} else {
		// Show timer
		bossPhaseTimeLeft->setVisible(true);
		bossPhaseHealthBar->setVisible(false);
		bossPhaseTimeLeft->setText((boost::format("%.2f") % (bossNextPhaseStartTime - registry.get<EnemyComponent>(boss).getTimeSinceLastPhase())).str());

		bossNextPhaseStartTime = std::dynamic_pointer_cast<TimeBasedEnemyPhaseStartCondition>(nextPhaseStartCondition)->getTime();

		// Set timer position depending on location of player
		if (registry.get<PositionComponent>(registry.attachee<PlayerTag>()).getX() + registry.get<HitboxComponent>(registry.attachee<PlayerTag>()).getX() < MAP_WIDTH/2.0f) {
			bossPhaseTimeLeft->setPosition(0, windowHeight - bossPhaseTimeLeft->getSize().y);
		} else {
			bossPhaseTimeLeft->setPosition(guiRegionX - bossPhaseTimeLeft->getSize().x, windowHeight - bossPhaseTimeLeft->getSize().y);
		}
	}
}

void GameInstance::onBossDespawn(uint32_t boss) {
	bossLabel->setVisible(false);
	bossPhaseTimeLeft->setVisible(false);
	bossPhaseHealthBar->setVisible(false);

	// Resume playing the level music, if any
	if (currentLevelMusic) {
		levelPack->playMusic(currentLevelMusic, currentLevel->getMusicSettings());
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
	auto& playerTag = registry.assign<PlayerTag>(entt::tag_t{}, player, registry, *levelPack, player, params.getSpeed(), params.getFocusedSpeed(), params.getInvulnerabilityTime(),
		params.getPowerTiers(), params.getHurtSound(), params.getDeathSound(), params.getInitialBombs(), params.getMaxBombs(), params.getBombInvincibilityTime());
	auto& health = registry.assign<HealthComponent>(player, params.getInitialHealth(), params.getMaxHealth());
	// Hitbox temporarily at 0, 0 until an Animatable is assigned to the player later
	registry.assign<HitboxComponent>(player, LOCK_ROTATION, params.getHitboxRadius(), 0, 0);
	registry.assign<PositionComponent>(player, PLAYER_SPAWN_X - params.getHitboxPosX(), PLAYER_SPAWN_Y - params.getHitboxPosY());
	registry.assign<SpriteComponent>(player, PLAYER_LAYER, 0);

	health.getHPChangeSignal()->sink().connect<GameInstance, &GameInstance::onPlayerHPChange>(this);
	playerTag.getPowerChangeSignal()->sink().connect<GameInstance, &GameInstance::onPlayerPowerLevelChange>(this);
	playerTag.getBombCountChangeSignal()->sink().connect<GameInstance, &GameInstance::onPlayerBombCountChange>(this);
	registry.get<LevelManagerTag>().getEnemySpawnSignal()->sink().connect<GameInstance, &GameInstance::onEnemySpawn>(this);
}