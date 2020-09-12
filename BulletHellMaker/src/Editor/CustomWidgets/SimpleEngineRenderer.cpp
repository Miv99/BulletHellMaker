#include <Editor/CustomWidgets/SimpleEngineRenderer.h>

SimpleEngineRenderer::SimpleEngineRenderer(sf::RenderWindow& parentWindow, bool userControlledView, bool useDebugRenderSystem) : parentWindow(parentWindow),
paused(true), userControlledView(userControlledView), useDebugRenderSystem(useDebugRenderSystem) {
	audioPlayer = std::make_unique<AudioPlayer>();
	queue = std::make_unique<EntityCreationQueue>(registry);

	if (userControlledView) {
		viewController = std::make_unique<ViewController>(parentWindow, true, false);
	}
	viewFromViewController.setCenter(MAP_WIDTH / 2.0f, MAP_HEIGHT / 2.0f);

	connect("PositionChanged", [&]() {
		updateWindowView();
	});
	connect("SizeChanged", [&]() {
		updateWindowView();
	});
}

std::shared_ptr<EditorPlayer> SimpleEngineRenderer::getPlayer() {
	return levelPack->getGameplayPlayer();
}

void SimpleEngineRenderer::updateWindowView() {
	if (!renderSystem) {
		return;
	}

	auto windowSize = parentWindow.getSize();
	auto size = getSize();
	float sizeRatio = size.x / (float)size.y;
	sf::Vector2u resolution = renderSystem->getResolution();
	float playAreaViewRatio = resolution.x / (float)resolution.y;

	float viewWidth, viewHeight;
	if (sizeRatio > playAreaViewRatio) {
		viewHeight = resolution.y;
		viewWidth = resolution.y * size.x / (float)size.y;
		float viewX = -(viewWidth - resolution.x) / 2.0f;
		float viewY = 0;
		viewFloatRect = sf::FloatRect(0, viewY, viewWidth, viewHeight);
	} else {
		viewWidth = resolution.x;
		viewHeight = resolution.x * size.y / (float)size.x;
		float viewX = 0;
		float viewY = -(viewHeight - resolution.y) / 2.0f;
		viewFloatRect = sf::FloatRect(0, viewY, viewWidth, viewHeight);
	}

	if (viewController) {
		viewController->setOriginalViewSize(viewWidth, viewHeight);
	}

	float viewportX = getAbsolutePosition().x / windowSize.x;
	float viewportY = getAbsolutePosition().y / windowSize.y;
	float viewportWidth = getSize().x / windowSize.x;
	float viewportHeight = getSize().y / windowSize.y;
	viewportFloatRect = sf::FloatRect(viewportX, viewportY, viewportWidth, viewportHeight);

	sf::Vector2f oldCenter = viewFromViewController.getCenter();
	viewController->setViewZone(viewFromViewController, viewFloatRect);
	viewFromViewController.setViewport(viewportFloatRect);
	viewFromViewController.setCenter(oldCenter);
}

bool SimpleEngineRenderer::update(sf::Time elapsedTime) {
	bool ret = tgui::Panel::update(elapsedTime);

	return viewController->update(viewFromViewController, elapsedTime.asSeconds()) || ret;
}

void SimpleEngineRenderer::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	// LevelPackObjectPreviewWindow will draw the render system before calling this draw().
	// To make sure the render system stuff is visible, we can't draw this widget's underlying tgui::Panel.
	// However, there might be widgets in the panel, so draw only those widgets.

	for (auto widget : getWidgets()) {
		widget->draw(target, states);
	}
}

bool SimpleEngineRenderer::handleEvent(sf::Event event) {
	if (viewController) {
		return viewController->handleEvent(viewFromViewController, event);
	}
	return false;
}

void SimpleEngineRenderer::loadLevelPack(std::string name) {
	std::lock_guard<std::mutex> lock(registryMutex);
	registry.reset();

	levelPack = std::make_shared<LevelPack>(*audioPlayer, name);
	spriteLoader = levelPack->createSpriteLoader();

	movementSystem = std::make_unique<MovementSystem>(*queue, *spriteLoader, registry);
	debugRenderSystem = std::make_unique<DebugRenderSystem>(registry, parentWindow, *spriteLoader, 1.0f);
	renderSystem = std::make_unique<RenderSystem>(registry, parentWindow, *spriteLoader, 1.0f);
	collisionSystem = std::make_unique<CollisionSystem>(*levelPack, *queue, *spriteLoader, registry, MAP_WIDTH, MAP_HEIGHT);
	despawnSystem = std::make_unique<DespawnSystem>(registry);
	enemySystem = std::make_unique<EnemySystem>(*queue, *spriteLoader, *levelPack, registry);
	spriteAnimationSystem = std::make_unique<SpriteAnimationSystem>(*spriteLoader, registry);
	shadowTrailSystem = std::make_unique<ShadowTrailSystem>(*queue, registry);
	playerSystem = std::make_unique<PlayerSystem>(*levelPack, *queue, *spriteLoader, registry);
	collectibleSystem = std::make_unique<CollectibleSystem>(*queue, registry, *levelPack, MAP_WIDTH, MAP_HEIGHT);

	debugRenderSystem->getOnResolutionChange()->sink().connect<SimpleEngineRenderer, &SimpleEngineRenderer::updateWindowView>(this);
	renderSystem->getOnResolutionChange()->sink().connect<SimpleEngineRenderer, &SimpleEngineRenderer::updateWindowView>(this);
	updateWindowView();
}

void SimpleEngineRenderer::loadLevel(int levelIndex) {
	if (!levelPack->hasLevel(levelIndex)) {
		throw "The level does not exist";
	}
	loadLevel(levelPack->getGameplayLevel(levelIndex));
}

void SimpleEngineRenderer::loadLevel(std::shared_ptr<Level> level) {
	std::lock_guard<std::mutex> lock(registryMutex);

	// Remove all existing entities from the registry
	registry.reset();
	reserveMemory(registry, INITIAL_EDITOR_ENTITY_RESERVATION);

	// Create the level manager
	registry.reserve<LevelManagerTag>(1);
	registry.reserve(registry.alive() + 1);
	uint32_t levelManager = registry.create();
	auto& levelManagerTag = registry.assign<LevelManagerTag>(entt::tag_t{}, levelManager, &(*levelPack), level);

	// Create the player
	auto params = getPlayer();
	registry.reserve(1);
	registry.reserve<PlayerTag>(1);
	registry.reserve<AnimatableSetComponent>(1);
	registry.reserve<HealthComponent>(1);
	registry.reserve<HitboxComponent>(1);
	registry.reserve<PositionComponent>(1);
	registry.reserve<SpriteComponent>(1);
	auto player = registry.create();
	registry.assign<AnimatableSetComponent>(player);
	auto& playerTag = registry.assign<PlayerTag>(entt::tag_t{}, player, registry, *levelPack, player, params->getSpeed(), params->getFocusedSpeed(), params->getInvulnerabilityTime(),
		params->getPowerTiers(), params->getHurtSound(), params->getDeathSound(), params->getInitialBombs(), params->getMaxBombs(), params->getBombInvincibilityTime());
	auto& health = registry.assign<HealthComponent>(player, params->getInitialHealth(), params->getMaxHealth());
	// Hitbox temporarily at 0, 0 until an Animatable is assigned to the player later
	registry.assign<HitboxComponent>(player, ROTATION_TYPE::LOCK_ROTATION, params->getHitboxRadius(), 0, 0);
	registry.assign<PositionComponent>(player, playerSpawnX - params->getHitboxPosX(), playerSpawnY - params->getHitboxPosY());
	registry.assign<SpriteComponent>(player, PLAYER_LAYER, 0);

	if (invinciblePlayer) {
		playerTag.setIsInvincible(true);
	}

	// Play level music
	levelPack->playMusic(level->getMusicSettings());

	// Set the background
	std::shared_ptr<sf::Texture> background = spriteLoader->getBackground(level->getBackgroundFileName());

	renderSystem->setBackground(background);
	renderSystem->setBackgroundScrollSpeedX(level->getBackgroundScrollSpeedX());
	renderSystem->setBackgroundScrollSpeedY(level->getBackgroundScrollSpeedY());
	renderSystem->setBackgroundTextureWidth(level->getBackgroundTextureWidth());
	renderSystem->setBackgroundTextureHeight(level->getBackgroundTextureHeight());

	debugRenderSystem->setBackground(background);
	debugRenderSystem->setBackgroundScrollSpeedX(level->getBackgroundScrollSpeedX());
	debugRenderSystem->setBackgroundScrollSpeedY(level->getBackgroundScrollSpeedY());
	debugRenderSystem->setBackgroundTextureWidth(level->getBackgroundTextureWidth());
	debugRenderSystem->setBackgroundTextureHeight(level->getBackgroundTextureHeight());
}

void SimpleEngineRenderer::pause() {
	paused = true;
}

void SimpleEngineRenderer::unpause() {
	paused = false;
}

void SimpleEngineRenderer::resetCamera() {
	viewFromViewController.setCenter(MAP_WIDTH / 2.0f, MAP_HEIGHT / 2.0f);
}

void SimpleEngineRenderer::physicsUpdate(float deltaTime) {
	if (!paused) {
		std::lock_guard<std::mutex> lock(registryMutex);
		deltaTime *= timeMultiplier;

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

void SimpleEngineRenderer::renderUpdate(float deltaTime) {
	std::lock_guard<std::mutex> lock(registryMutex);
	deltaTime *= timeMultiplier;

	// Viewport is set here because tgui::Gui's draw function changes it right before renderSystem is updated or something
	sf::View originalView = parentWindow.getView();
	if (viewController) {
		parentWindow.setView(viewFromViewController);
	} else {
		sf::View view(viewFloatRect);
		view.setViewport(viewportFloatRect);
		parentWindow.setView(view);
	}
	if (!paused) {
		spriteAnimationSystem->update(deltaTime);
	}
	if (useDebugRenderSystem) {
		debugRenderSystem->update(deltaTime);
	} else {
		renderSystem->update(deltaTime);
	}
	parentWindow.setView(originalView);
}

void SimpleEngineRenderer::setUseDebugRenderSystem(bool useDebugRenderSystem) {
	this->useDebugRenderSystem = useDebugRenderSystem;
}

void SimpleEngineRenderer::setTimeMultiplier(float timeMultiplier) {
	this->timeMultiplier = timeMultiplier;
}

void SimpleEngineRenderer::setPlayerSpawn(float x, float y) {
	playerSpawnX = x;
	playerSpawnY = y;
}

void SimpleEngineRenderer::setInvinciblePlayer(bool invinciblePlayer) {
	this->invinciblePlayer = invinciblePlayer;
	if (registry.has<PlayerTag>()) {
		registry.get<PlayerTag>().setIsInvincible(invinciblePlayer);
	}
}

bool SimpleEngineRenderer::getUseDebugRenderSystem() const {
	return useDebugRenderSystem;
}

float SimpleEngineRenderer::getTimeMultiplier() const {
	return timeMultiplier;
}