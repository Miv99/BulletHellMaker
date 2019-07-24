#include "EditorWindow.h"
#include <algorithm>

EditorWindow::EditorWindow(std::shared_ptr<std::mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize, bool letterboxingEnabled, float renderInterval) :
	tguiMutex(tguiMutex), windowTitle(windowTitle), windowWidth(width), windowHeight(height), scaleWidgetsOnResize(scaleWidgetsOnResize), letterboxingEnabled(letterboxingEnabled), renderInterval(renderInterval) {
	std::lock_guard<std::mutex> lock(*tguiMutex);
	gui = std::make_shared<tgui::Gui>();
}

void EditorWindow::start() {
	if (!window) {
		// SFML requires the RenderWindow to be created in the thread

		std::lock_guard<std::mutex> lock(*tguiMutex);
		window = std::make_shared<sf::RenderWindow>(sf::VideoMode(windowWidth, windowHeight), windowTitle, sf::Style::Default);
		window->setKeyRepeatEnabled(false);
		window->setActive(true);
		gui->setTarget(*window);

		updateWindowView(window->getSize().x, window->getSize().y);
	}

	sf::Clock deltaClock;

	// Main loop
	while (window->isOpen()) {
		// While behind in render updates, do physics updates
		float timeSinceLastRender = 0;
		while (timeSinceLastRender < renderInterval) {
			sf::Event event;
			while (window->pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					window->close();
				} else if (event.type == sf::Event::Resized) {
					updateWindowView(event.size.width, event.size.height);
				} else {
					if (popup) {
						if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
							// When mouse is released, remove the pop-up menu

							closePopupWidget();
						}
					}
					handleEvent(event);
				}
			}

			float dt = std::min(MAX_PHYSICS_DELTA_TIME, deltaClock.restart().asSeconds());
			physicsUpdate(dt);

			timeSinceLastRender += dt;
		}

		window->clear();
		render(timeSinceLastRender);
		if (renderSignal) {
			renderSignal->publish(timeSinceLastRender);
		}
		window->display();
	}
}

void EditorWindow::close() {
	window->close();
}

void EditorWindow::addPopupWidget(std::shared_ptr<tgui::Container> popupContainer, std::shared_ptr<tgui::Widget> popup) {
	if (this->popup) {
		closePopupWidget();
	}
	this->popup = popup;
	this->popupContainer = popupContainer;
	popupContainer->add(popup);
}

void EditorWindow::closePopupWidget() {
	popupContainer->remove(popup);
	popup = nullptr;
	popupContainer = nullptr;
}

void EditorWindow::updateWindowView(int windowWidth, int windowHeight) {
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	
	if (letterboxingEnabled) {
		sf::View view = window->getView();

		// Compares the aspect ratio of the window to the aspect ratio of the view,
		// and sets the view's viewport accordingly in order to archieve a letterbox effect.
		// A new view (with a new viewport set) is returned.
		float windowRatio = windowWidth / (float)windowHeight;
		float viewRatio = view.getSize().x / (float)view.getSize().y;
		float sizeX = 1;
		float sizeY = 1;
		float posX = 0;
		float posY = 0;

		bool horizontalSpacing = true;
		if (windowRatio < viewRatio)
			horizontalSpacing = false;

		// If horizontalSpacing is true, the black bars will appear on the left and right side.
		// Otherwise, the black bars will appear on the top and bottom.
		if (horizontalSpacing) {
			sizeX = viewRatio / windowRatio;
			posX = (1 - sizeX) / 2.f;
		} else {
			sizeY = windowRatio / viewRatio;
			posY = (1 - sizeY) / 2.f;
		}

		view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));
		window->setView(view);
	}
	if (!scaleWidgetsOnResize) {
		gui->setView(sf::View(sf::Vector2f(windowWidth / 2.0f, windowHeight / 2.0f), sf::Vector2f(windowWidth, windowHeight)));
	}

	if (resizeSignal) {
		resizeSignal->publish(windowWidth, windowHeight);
	}
}

std::shared_ptr<entt::SigH<void(float)>> EditorWindow::getRenderSignal() {
	if (!renderSignal) {
		renderSignal = std::make_shared<entt::SigH<void(float)>>();
	}
	return renderSignal;
}

std::shared_ptr<entt::SigH<void(int, int)>> EditorWindow::getResizeSignal() {
	if (!resizeSignal) {
		resizeSignal = std::make_shared<entt::SigH<void(int, int)>>();
	}
	return resizeSignal;
}

void EditorWindow::physicsUpdate(float deltaTime) {
}

void EditorWindow::render(float deltaTime) {
	std::lock_guard<std::mutex> lock(*tguiMutex);
	gui->draw();
}

void EditorWindow::handleEvent(sf::Event event) {
	std::lock_guard<std::mutex> lock(*tguiMutex);
	gui->handleEvent(event);
}

void UndoableEditorWindow::handleEvent(sf::Event event) {
	EditorWindow::handleEvent(event);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Z) {
			undoStack.undo();
		} else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Y) {
			undoStack.redo();
		}
	}
}

GameplayTestWindow::GameplayTestWindow(std::shared_ptr<LevelPack> levelPack, std::shared_ptr<SpriteLoader> spriteLoader, std::shared_ptr<std::mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize, bool letterboxingEnabled, float renderInterval) :
	undoStack(UndoStack(UNDO_STACK_MAX)), levelPack(levelPack), spriteLoader(spriteLoader), UndoableEditorWindow(tguiMutex, windowTitle, width, height, undoStack, scaleWidgetsOnResize, letterboxingEnabled, renderInterval) {
	audioPlayer = std::make_unique<AudioPlayer>();
	queue = std::make_unique<EntityCreationQueue>(registry);
	movementSystem = std::make_unique<MovementSystem>(*queue, *spriteLoader, registry);
	//TODO: these numbers should come from settings like in GameInstance
	renderSystem = std::make_unique<RenderSystem>(registry, *window, 1024, 768);
	renderSystem->loadLevelRenderSettings(nullptr);
	collisionSystem = std::make_unique<CollisionSystem>(*levelPack, *queue, *spriteLoader, registry, MAP_WIDTH, MAP_HEIGHT);
	despawnSystem = std::make_unique<DespawnSystem>(registry);
	enemySystem = std::make_unique<EnemySystem>(*queue, *spriteLoader, *levelPack, registry);
	spriteAnimationSystem = std::make_unique<SpriteAnimationSystem>(*spriteLoader, registry);
	shadowTrailSystem = std::make_unique<ShadowTrailSystem>(*queue, registry);
	playerSystem = std::make_unique<PlayerSystem>(*levelPack, *queue, *spriteLoader, registry);
	collectibleSystem = std::make_unique<CollectibleSystem>(*queue, registry, *levelPack, MAP_WIDTH, MAP_HEIGHT);

	leftPanel = tgui::ScrollablePanel::create();
	entityPlaceholdersListLabel = tgui::Label::create();
	entityPlaceholdersList = tgui::ListBox::create();
	newEnemyPlaceholder = tgui::Button::create();
	deleteEnemyPlaceholder = tgui::Button::create();

	rightPanel = tgui::ScrollablePanel::create();
	enemyPlaceholderXLabel = tgui::Label::create();
	enemyPlaceholderX = std::make_shared<NumericalEditBoxWithLimits>();
	enemyPlaceholderYLabel = tgui::Label::create();
	enemyPlaceholderY = std::make_shared<NumericalEditBoxWithLimits>();
	enemyPlaceholderManualSet = tgui::Button::create();
	setEnemyPlaceholderTestMode = tgui::Button::create();
	testModeIDLabel = tgui::Label::create();
	testModeID = tgui::ListBox::create();

	entityPlaceholdersListLabel->setText("Entities");
	newEnemyPlaceholder->setText("New enemy");
	deleteEnemyPlaceholder->setText("Delete enemy");
	enemyPlaceholderXLabel->setText("X");
	enemyPlaceholderYLabel->setText("Y");
	enemyPlaceholderManualSet->setText("Manual set");
	setEnemyPlaceholderTestMode->setText("Set test mode");

	entityPlaceholdersListLabel->setTextSize(TEXT_SIZE);
	newEnemyPlaceholder->setTextSize(TEXT_SIZE);
	deleteEnemyPlaceholder->setTextSize(TEXT_SIZE);
	enemyPlaceholderXLabel->setTextSize(TEXT_SIZE);
	enemyPlaceholderYLabel->setTextSize(TEXT_SIZE);
	enemyPlaceholderManualSet->setTextSize(TEXT_SIZE);
	setEnemyPlaceholderTestMode->setTextSize(TEXT_SIZE);
	testModeIDLabel->setTextSize(TEXT_SIZE);
	entityPlaceholdersList->setTextSize(TEXT_SIZE);
	enemyPlaceholderX->setTextSize(TEXT_SIZE);
	enemyPlaceholderY->setTextSize(TEXT_SIZE);
	testModeID->setTextSize(TEXT_SIZE);

	leftPanel->add(entityPlaceholdersListLabel);
	leftPanel->add(entityPlaceholdersList);
	leftPanel->add(newEnemyPlaceholder);
	leftPanel->add(deleteEnemyPlaceholder);
	rightPanel->add(enemyPlaceholderXLabel);
	rightPanel->add(enemyPlaceholderX);
	rightPanel->add(enemyPlaceholderYLabel);
	rightPanel->add(enemyPlaceholderY);
	rightPanel->add(enemyPlaceholderManualSet);
	rightPanel->add(setEnemyPlaceholderTestMode);
	rightPanel->add(testModeIDLabel);
	rightPanel->add(testModeID);

	getGui()->add(leftPanel);
	getGui()->add(rightPanel);
}

void GameplayTestWindow::handleEvent(sf::Event event) {
	UndoableEditorWindow::handleEvent(event);
}

void GameplayTestWindow::physicsUpdate(float deltaTime) {
	UndoableEditorWindow::physicsUpdate(deltaTime);

	if (!paused) {
		audioPlayer->update(deltaTime);

		collisionSystem->update(deltaTime);
		queue->executeAll();

		//registry.get<LevelManagerTag>().update(*queue, *spriteLoader, registry, deltaTime);
		//queue->executeAll();

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

void GameplayTestWindow::render(float deltaTime) {
	UndoableEditorWindow::render(deltaTime);

	if (!paused) {
		spriteAnimationSystem->update(deltaTime);
	}
	renderSystem->update(deltaTime);
}

void GameplayTestWindow::updateWindowView(int width, int height) {
	UndoableEditorWindow::updateWindowView(width, height);

	const float leftPanelWidth = width * 0.25f;
	leftPanel->setPosition(0, 0);
	leftPanel->setSize(leftPanelWidth, height);
	entityPlaceholdersListLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	entityPlaceholdersList->setPosition(GUI_PADDING_X, tgui::bindBottom(entityPlaceholdersListLabel) + GUI_PADDING_Y);
	newEnemyPlaceholder->setPosition(GUI_PADDING_X, tgui::bindBottom(entityPlaceholdersList));
	deleteEnemyPlaceholder->setPosition(tgui::bindRight(newEnemyPlaceholder) + GUI_PADDING_X, tgui::bindTop(newEnemyPlaceholder));
	entityPlaceholdersList->setSize(width - GUI_PADDING_X * 2, height * 0.5f);
	newEnemyPlaceholder->setSize(100, TEXT_BUTTON_HEIGHT);
	deleteEnemyPlaceholder->setSize(100, TEXT_BUTTON_HEIGHT);

	const float rightPanelWidth = width * 0.25f;
	rightPanel->setPosition(width - rightPanelWidth, 0);
	rightPanel->setSize(rightPanelWidth, height);
	enemyPlaceholderXLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	enemyPlaceholderX->setPosition(GUI_PADDING_X, tgui::bindBottom(enemyPlaceholderXLabel) + GUI_PADDING_Y);
	enemyPlaceholderYLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(enemyPlaceholderX) + GUI_PADDING_Y);
	enemyPlaceholderY->setPosition(GUI_PADDING_X, tgui::bindBottom(enemyPlaceholderYLabel) + GUI_PADDING_Y);
	enemyPlaceholderManualSet->setPosition(GUI_PADDING_X, tgui::bindBottom(enemyPlaceholderY) + GUI_PADDING_Y);
	setEnemyPlaceholderTestMode->setPosition(GUI_PADDING_X, tgui::bindBottom(enemyPlaceholderManualSet) + GUI_PADDING_Y);
	testModeIDLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(setEnemyPlaceholderTestMode) + GUI_PADDING_Y);
	testModeID->setPosition(GUI_PADDING_X, tgui::bindBottom(testModeIDLabel) + GUI_PADDING_Y);
	enemyPlaceholderX->setSize(rightPanelWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	enemyPlaceholderY->setSize(rightPanelWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	enemyPlaceholderManualSet->setSize(100, TEXT_BUTTON_HEIGHT);
	setEnemyPlaceholderTestMode->setSize(100, TEXT_BUTTON_HEIGHT);
	testModeID->setSize(rightPanelWidth - GUI_PADDING_X * 2, height * 0.5f);
}
