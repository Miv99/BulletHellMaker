#include "EditorWindow.h"
#include "Components.h"
#include <algorithm>
#include <iostream>

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
		onRenderWindowInitialization();
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
	startAndEndTest = tgui::Button::create();
	toggleBottomPanelDisplay = tgui::Button::create();

	rightPanel = tgui::ScrollablePanel::create();
	entityPlaceholderXLabel = tgui::Label::create();
	entityPlaceholderX = std::make_shared<NumericalEditBoxWithLimits>();
	entityPlaceholderYLabel = tgui::Label::create();
	entityPlaceholderY = std::make_shared<NumericalEditBoxWithLimits>();
	entityPlaceholderManualSet = tgui::Button::create();
	setEnemyPlaceholderTestMode = tgui::Button::create();
	testModeIDLabel = tgui::Label::create();
	testModeID = tgui::ListBox::create();
	testModePopup = tgui::ListBox::create();

	bottomPanel = tgui::ScrollablePanel::create();
	logs = tgui::Label::create();

	entityPlaceholdersListLabel->setText("Entities");
	newEnemyPlaceholder->setText("New enemy");
	deleteEnemyPlaceholder->setText("Delete enemy");
	entityPlaceholderXLabel->setText("X");
	entityPlaceholderYLabel->setText("Y");
	entityPlaceholderManualSet->setText("Manual set");
	setEnemyPlaceholderTestMode->setText("Set test mode");
	startAndEndTest->setText("Start test");
	toggleBottomPanelDisplay->setText("Show logs");

	entityPlaceholdersListLabel->setTextSize(TEXT_SIZE);
	newEnemyPlaceholder->setTextSize(TEXT_SIZE);
	deleteEnemyPlaceholder->setTextSize(TEXT_SIZE);
	entityPlaceholderXLabel->setTextSize(TEXT_SIZE);
	entityPlaceholderYLabel->setTextSize(TEXT_SIZE);
	entityPlaceholderManualSet->setTextSize(TEXT_SIZE);
	setEnemyPlaceholderTestMode->setTextSize(TEXT_SIZE);
	testModeIDLabel->setTextSize(TEXT_SIZE);
	entityPlaceholdersList->setTextSize(TEXT_SIZE);
	entityPlaceholderX->setTextSize(TEXT_SIZE);
	entityPlaceholderY->setTextSize(TEXT_SIZE);
	testModeID->setTextSize(TEXT_SIZE);
	testModePopup->setTextSize(TEXT_SIZE);
	startAndEndTest->setTextSize(TEXT_SIZE);
	toggleBottomPanelDisplay->setTextSize(TEXT_SIZE);

	testModePopup->addItem("Enemy", std::to_string(static_cast<int>(EnemyEntityPlaceholder::ENEMY)));
	testModePopup->addItem("Enemy phase", std::to_string(static_cast<int>(EnemyEntityPlaceholder::PHASE)));
	testModePopup->addItem("Attack pattern", std::to_string(static_cast<int>(EnemyEntityPlaceholder::ATTACK_PATTERN)));
	testModePopup->addItem("Attack", std::to_string(static_cast<int>(EnemyEntityPlaceholder::ATTACK)));
	testModePopup->setItemHeight(20);
	testModePopup->setSize(150, testModePopup->getItemHeight() * testModePopup->getItemCount());
	testModePopup->setPosition(tgui::bindLeft(setEnemyPlaceholderTestMode), tgui::bindTop(setEnemyPlaceholderTestMode));

	entityPlaceholdersList->connect("ItemSelected", [&](std::string item, std::string id) {
		if (ignoreSignal) return;
		if (id == "") {
			deselectPlaceholder();
		} else {
			if (std::stoi(id) == playerPlaceholder->getID()) {
				selectPlaceholder(playerPlaceholder);
			} else {
				selectPlaceholder(enemyPlaceholders[std::stoi(id)]);
			}
		}
	});
	newEnemyPlaceholder->connect("Pressed", [&]() {
		setPlacingNewEnemy(true);
		setManuallySettingPlaceholderPosition(selectedPlaceholder, false);
	});
	deleteEnemyPlaceholder->connect("Pressed", [this, &selectedPlaceholder = this->selectedPlaceholder]() {
		std::shared_ptr<EnemyEntityPlaceholder> enemy = std::dynamic_pointer_cast<EnemyEntityPlaceholder>(selectedPlaceholder);
		undoStack.execute(UndoableCommand(
			[this, selectedPlaceholder]() {
			deletePlaceholder(selectedPlaceholder->getID());
		},
			[this, enemy]() {
			int id = enemy->getID();
			mostRecentNewEnemyPlaceholderID = id;
			enemy->spawnVisualEntity();
			enemyPlaceholders[id] = enemy;
			updateEntityPlaceholdersList();
		}));
	});
	entityPlaceholderManualSet->connect("Pressed", [&]() {
		setPlacingNewEnemy(false);
		setManuallySettingPlaceholderPosition(selectedPlaceholder, true);
	});
	entityPlaceholderX->getOnValueSet()->sink().connect<GameplayTestWindow, &GameplayTestWindow::onEntityPlaceholderXValueSet>(this);
	entityPlaceholderY->getOnValueSet()->sink().connect<GameplayTestWindow, &GameplayTestWindow::onEntityPlaceholderYValueSet>(this);
	testModeID->connect("ItemSelected", [&](std::string itemName, std::string itemID) {
		if (itemID != "") {
			assert(dynamic_cast<EnemyEntityPlaceholder*>(selectedPlaceholder.get()) != nullptr);
			auto enemyPlaceholder = dynamic_cast<EnemyEntityPlaceholder*>(selectedPlaceholder.get());
			enemyPlaceholder->setTestModeID(std::stoi(itemID));
			updateEntityPlaceholdersList();
		}
	});
	setEnemyPlaceholderTestMode->connect("Pressed", [&]() {
		addPopupWidget(rightPanel, testModePopup);
	});
	testModePopup->connect("ItemSelected", [&](std::string itemName, std::string itemID) {
		if (ignoreSignal) return;

		assert(dynamic_cast<EnemyEntityPlaceholder*>(selectedPlaceholder.get()) != nullptr);
		auto enemyPlaceholder = dynamic_cast<EnemyEntityPlaceholder*>(selectedPlaceholder.get());
		enemyPlaceholder->setTestMode(static_cast<EnemyEntityPlaceholder::TEST_MODE>(std::stoi(itemID)));
		// Load testModeID widget's items
		selectPlaceholder(selectedPlaceholder);
	});
	startAndEndTest->connect("Pressed", [&]() {
		if (testInProgress) {
			endGameplayTest();
		} else {
			runGameplayTest();
		}
	});
	toggleBottomPanelDisplay->connect("Pressed", [&]() {
		toggleLogsDisplay();
	});

	leftPanel->add(entityPlaceholdersListLabel);
	leftPanel->add(entityPlaceholdersList);
	leftPanel->add(newEnemyPlaceholder);
	leftPanel->add(deleteEnemyPlaceholder);
	leftPanel->add(startAndEndTest);
	leftPanel->add(toggleBottomPanelDisplay);
	rightPanel->add(entityPlaceholderXLabel);
	rightPanel->add(entityPlaceholderX);
	rightPanel->add(entityPlaceholderYLabel);
	rightPanel->add(entityPlaceholderY);
	rightPanel->add(entityPlaceholderManualSet);
	rightPanel->add(setEnemyPlaceholderTestMode);
	rightPanel->add(testModeIDLabel);
	rightPanel->add(testModeID);
	bottomPanel->add(logs);

	getGui()->add(leftPanel);
	getGui()->add(rightPanel);
	getGui()->add(bottomPanel);

	// --------------------------------
	playerPlaceholder = std::make_shared<PlayerEntityPlaceholder>(nextPlaceholderID, registry, *spriteLoader, *levelPack);
	nextPlaceholderID++;
	playerPlaceholder->moveTo(PLAYER_SPAWN_X, PLAYER_SPAWN_Y);
	playerPlaceholder->spawnVisualEntity();
	// ---------------------------------
	movingEnemyPlaceholderCursor = spriteLoader->getSprite("Enemy Placeholder", "Default");
	movingPlayerPlaceholderCursor = spriteLoader->getSprite("Player Placeholder", "Default");
	// ---------------------------------
	// Create the level manager
	registry.reserve<LevelManagerTag>(1);
	registry.reserve(registry.alive() + 1);
	uint32_t levelManager = registry.create();
	auto level = std::make_shared<Level>();
	//TODO: default level stuff (just the health/points/power packs, bomb items)
	auto& levelManagerTag = registry.assign<LevelManagerTag>(entt::tag_t{}, levelManager, &(*levelPack), level);
	// ----------------------------------
	toggleLogsDisplay();
	updateEntityPlaceholdersList();
}

void GameplayTestWindow::handleEvent(sf::Event event) {
	UndoableEditorWindow::handleEvent(event);

	if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::L) {
			toggleLogsDisplay();
		}
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
		if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::Dash) {
				setCameraZoom(std::max(0.2f, cameraZoom - 0.2f));
			} else if (event.key.code == sf::Keyboard::Equal) {
				setCameraZoom(std::min(4.0f, cameraZoom + 0.2f));
			}
		} else if (event.type == sf::Event::MouseWheelScrolled) {
			if (event.mouseWheelScroll.delta < 0) {
				setCameraZoom(std::max(0.2f, cameraZoom - 0.2f));
			} else if (event.mouseWheelScroll.delta > 0) {
				setCameraZoom(std::min(4.0f, cameraZoom + 0.2f));
			}
		}
	} else if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Left) {
			float windowWidth = window->getSize().x;
			// Check if mouse is not in the gui
			if ((!leftPanel->isVisible() || event.mouseButton.x > windowWidth * LEFT_PANEL_WIDTH) && (!rightPanel->isVisible() || event.mouseButton.x < windowWidth * (1 - RIGHT_PANEL_WIDTH))) {
				onGameplayAreaMouseClick(event.mouseButton.x, event.mouseButton.y);
			}
		} else if (event.mouseButton.button == sf::Mouse::Right) {
			setPlacingNewEnemy(false);
			setManuallySettingPlaceholderPosition(selectedPlaceholder, false);
		} else if (event.mouseButton.button == sf::Mouse::Middle) {
			draggingCamera = true;
			previousCameraDragCoordsX = event.mouseButton.x;
			previousCameraDragCoordsY = event.mouseButton.y;
		}
	} else if (event.type == sf::Event::MouseMoved) {
		if (draggingCamera) {
			// Move camera depending on difference in world coordinates between event.mouseMove.x/y and previousCameraDragCoordsX/Y
			sf::Vector2f diff = window->mapPixelToCoords(sf::Vector2i(previousCameraDragCoordsX, previousCameraDragCoordsY)) - window->mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
			moveCamera(diff.x, diff.y);
		} else if (draggingPlaceholder) {
			// Move selected placeholder depending on difference in world coordinates between event.mouseMove.x/y and previousPlaceholderDragCoordsX/Y
			sf::Vector2f diff = window->mapPixelToCoords(sf::Vector2i(previousPlaceholderDragCoordsX, previousPlaceholderDragCoordsY)) - window->mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
			selectedPlaceholder->moveTo(selectedPlaceholder->getX() - diff.x, selectedPlaceholder->getY() + diff.y);
			if (selectedPlaceholder == this->selectedPlaceholder) {
				entityPlaceholderX->setValue(selectedPlaceholder->getX());
				entityPlaceholderY->setValue(selectedPlaceholder->getY());
			}
		}

		if (draggingCamera) {
			previousCameraDragCoordsX = event.mouseMove.x;
			previousCameraDragCoordsY = event.mouseMove.y;
		}
		if (draggingPlaceholder) {
			previousPlaceholderDragCoordsX = event.mouseMove.x;
			previousPlaceholderDragCoordsY = event.mouseMove.y;
		}
	} else if (event.type == sf::Event::MouseButtonReleased) {
		float windowWidth = window->getSize().x;
		if (draggingCamera && event.mouseButton.button == sf::Mouse::Middle) {
			draggingCamera = false;
		} else if (event.mouseButton.button == sf::Mouse::Left) {
			if ((!leftPanel->isVisible() || event.mouseButton.x > windowWidth * LEFT_PANEL_WIDTH) && (!rightPanel->isVisible() || event.mouseButton.x < windowWidth * (1 - RIGHT_PANEL_WIDTH))) {
				// Release event was from left mouse and in the gameplay area

				// Check if initial press was in gameplay area and in relative spatial proximity to mouse release
				float screenDist = std::sqrt((initialMousePressX - event.mouseButton.x)*(initialMousePressX - event.mouseButton.x) + (initialMousePressY - event.mouseButton.y)*(initialMousePressY - event.mouseButton.y));
				if (!justSelectedPlaceholder && screenDist < 15 && (!leftPanel->isVisible() || initialMousePressX > windowWidth * LEFT_PANEL_WIDTH) && (!rightPanel->isVisible() || initialMousePressX < windowWidth * (1 - RIGHT_PANEL_WIDTH))) {
					deselectPlaceholder();
				}
			}

			if (draggingPlaceholder) {
				draggingPlaceholder = false;

				if (selectedPlaceholder) {
					sf::Vector2f placeholderEndingPos(selectedPlaceholder->getX(), selectedPlaceholder->getY());
					undoStack.execute(UndoableCommand(
						[this, &selectedPlaceholder = this->selectedPlaceholder, placeholderEndingPos]() {
						selectedPlaceholder->moveTo(placeholderEndingPos.x, placeholderEndingPos.y);
						if (selectedPlaceholder == this->selectedPlaceholder) {
							entityPlaceholderX->setValue(selectedPlaceholder->getX());
							entityPlaceholderY->setValue(selectedPlaceholder->getY());
						}
					},
						[this, &selectedPlaceholder = this->selectedPlaceholder, &placeholderPosBeforeDragging = this->placeholderPosBeforeDragging]() {
						selectedPlaceholder->moveTo(placeholderPosBeforeDragging.x, placeholderPosBeforeDragging.y);
						if (selectedPlaceholder == this->selectedPlaceholder) {
							entityPlaceholderX->setValue(selectedPlaceholder->getX());
							entityPlaceholderY->setValue(selectedPlaceholder->getY());
						}
					}));
				}
			}

			justSelectedPlaceholder = false;
		}
	}
}

void GameplayTestWindow::physicsUpdate(float deltaTime) {
	UndoableEditorWindow::physicsUpdate(deltaTime);

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

void GameplayTestWindow::render(float deltaTime) {
	int xDirection = 0, yDirection = 0;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
		yDirection--;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
		yDirection++;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
		xDirection--;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
		xDirection++;
	}
	moveCamera(CAMERA_SPEED * xDirection * deltaTime / cameraZoom, CAMERA_SPEED * yDirection * deltaTime / cameraZoom);

	if (!paused) {
		spriteAnimationSystem->update(deltaTime);
	}
	renderSystem->update(deltaTime);

	UndoableEditorWindow::render(deltaTime);

	if (currentCursor) {
		sf::View oldView = window->getView();
		sf::View fixedView = sf::View(sf::FloatRect(0, -MAP_HEIGHT, MAP_WIDTH, MAP_HEIGHT));
		auto pos = sf::Mouse::getPosition(*window);
		currentCursor->setPosition(pos.x, pos.y - fixedView.getSize().y);
		auto oldCursorScale = currentCursor->getScale();

		window->setView(fixedView);
		currentCursor->setScale(oldCursorScale.x * cameraZoom, oldCursorScale.y * cameraZoom);
		window->draw(*currentCursor);

		window->setView(oldView);
		currentCursor->setScale(oldCursorScale);
	}
}

void GameplayTestWindow::updateWindowView(int width, int height) {
	UndoableEditorWindow::updateWindowView(width, height);

	const float leftPanelWidth = width * LEFT_PANEL_WIDTH;
	leftPanel->setPosition(0, 0);
	leftPanel->setSize(leftPanelWidth, height);
	entityPlaceholdersListLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	entityPlaceholdersList->setPosition(GUI_PADDING_X, tgui::bindBottom(entityPlaceholdersListLabel) + GUI_PADDING_Y);
	newEnemyPlaceholder->setPosition(GUI_PADDING_X, tgui::bindBottom(entityPlaceholdersList) + GUI_PADDING_Y);
	deleteEnemyPlaceholder->setPosition(GUI_PADDING_X, tgui::bindBottom(newEnemyPlaceholder) + GUI_PADDING_Y);
	startAndEndTest->setPosition(GUI_PADDING_X, tgui::bindBottom(deleteEnemyPlaceholder) + GUI_PADDING_Y * 2);
	toggleBottomPanelDisplay->setPosition(GUI_PADDING_X, tgui::bindBottom(startAndEndTest) + GUI_PADDING_Y);
	entityPlaceholdersList->setSize(leftPanelWidth - GUI_PADDING_X * 2, std::max(height * 0.75f, entityPlaceholdersList->getItemHeight() * 5.0f));
	newEnemyPlaceholder->setSize(std::max(leftPanelWidth - GUI_PADDING_X * 2, 100.0f), TEXT_BUTTON_HEIGHT);
	deleteEnemyPlaceholder->setSize(std::max(leftPanelWidth - GUI_PADDING_X * 2, 100.0f), TEXT_BUTTON_HEIGHT);
	startAndEndTest->setSize(std::max(leftPanelWidth - GUI_PADDING_X * 2, 100.0f), TEXT_BUTTON_HEIGHT * 2);
	toggleBottomPanelDisplay->setSize(std::max(leftPanelWidth - GUI_PADDING_X * 2, 100.0f), TEXT_BUTTON_HEIGHT);

	const float rightPanelWidth = width * RIGHT_PANEL_WIDTH;
	rightPanel->setPosition(width - rightPanelWidth, 0);
	rightPanel->setSize(rightPanelWidth, height);
	entityPlaceholderXLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	entityPlaceholderX->setPosition(GUI_PADDING_X, tgui::bindBottom(entityPlaceholderXLabel) + GUI_PADDING_Y);
	entityPlaceholderYLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(entityPlaceholderX) + GUI_PADDING_Y);
	entityPlaceholderY->setPosition(GUI_PADDING_X, tgui::bindBottom(entityPlaceholderYLabel) + GUI_PADDING_Y);
	entityPlaceholderManualSet->setPosition(GUI_PADDING_X, tgui::bindBottom(entityPlaceholderY) + GUI_PADDING_Y);
	setEnemyPlaceholderTestMode->setPosition(GUI_PADDING_X, tgui::bindBottom(entityPlaceholderManualSet) + GUI_PADDING_Y);
	testModeIDLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(setEnemyPlaceholderTestMode) + GUI_PADDING_Y);
	testModeID->setPosition(GUI_PADDING_X, tgui::bindBottom(testModeIDLabel) + GUI_PADDING_Y);
	entityPlaceholderX->setSize(rightPanelWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	entityPlaceholderY->setSize(rightPanelWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	entityPlaceholderManualSet->setSize(100, TEXT_BUTTON_HEIGHT);
	setEnemyPlaceholderTestMode->setSize(100, TEXT_BUTTON_HEIGHT);
	testModeID->setSize(rightPanelWidth - GUI_PADDING_X * 2, height * 0.5f);

	const float bottomPanelWidth = width - leftPanelWidth - rightPanelWidth;
	const float bottomPanelHeight = height * 0.25f;
	bottomPanel->setSize(bottomPanelWidth, bottomPanelHeight);
	bottomPanel->setPosition(tgui::bindRight(leftPanel), height - bottomPanelHeight);
	logs->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	logs->setSize(bottomPanelWidth - GUI_PADDING_X * 2, bottomPanelHeight - GUI_PADDING_Y);
	logs->setMaximumTextWidth(logs->getSize().x);
}

void GameplayTestWindow::onRenderWindowInitialization() {
	// RenderSystem is initialized here instead of in the constructor beause RenderSystem
	// requires a reference to the RenderWindow, but the RenderWindow is not initialized until
	// start() is called, which is after the constructor call

	window->setKeyRepeatEnabled(false);
	sf::View view(sf::FloatRect(0, -MAP_HEIGHT, MAP_WIDTH, MAP_HEIGHT));
	window->setView(view);
	updateWindowView(window->getSize().x, window->getSize().y);

	//TODO: these numbers should come from settings like in GameInstance
	renderSystem = std::make_unique<RenderSystem>(registry, *window, 1024, 768);
	renderSystem->loadLevelRenderSettings(nullptr);

	// Set the background
	std::shared_ptr<Level> level = levelPack->getLevel(0);
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

	deselectPlaceholder();
}

void GameplayTestWindow::onEntityPlaceholderXValueSet(float value) {
	float oldValue = selectedPlaceholder->getX();
	undoStack.execute(UndoableCommand(
		[this, &selectedPlaceholder = this->selectedPlaceholder, value]() {
		selectedPlaceholder->moveTo(value, selectedPlaceholder->getY());
	},
		[this, &selectedPlaceholder = this->selectedPlaceholder, oldValue]() {
		selectedPlaceholder->moveTo(oldValue, selectedPlaceholder->getY());
	}));
}

void GameplayTestWindow::onEntityPlaceholderYValueSet(float value) {
	float oldValue = selectedPlaceholder->getY();
	undoStack.execute(UndoableCommand(
		[this, &selectedPlaceholder = this->selectedPlaceholder, value]() {
		selectedPlaceholder->moveTo(selectedPlaceholder->getX(), value);
	},
		[this, &selectedPlaceholder = this->selectedPlaceholder, oldValue]() {
		selectedPlaceholder->moveTo(selectedPlaceholder->getX(), oldValue);
	}));
}

void GameplayTestWindow::moveCamera(float xOffset, float yOffset) {
	auto currentCenter = window->getView().getCenter();
	lookAt(currentCenter.x + xOffset, currentCenter.y + yOffset);
}

void GameplayTestWindow::lookAt(float x, float y) {
	sf::View view = window->getView();
	view.setCenter(x, y);
	window->setView(view);
}

void GameplayTestWindow::setCameraZoom(float zoom) {
	cameraZoom = zoom;
	sf::View view = window->getView();
	// Centered at negative y because SFML has (0, 0) at the top-left, and RenderSystem negates y-values so that (0, 0) in every other aspect of this game is bottom-left.
	view.setSize(MAP_WIDTH / zoom, MAP_HEIGHT / zoom);
	window->setView(view);
}

void GameplayTestWindow::onGameplayAreaMouseClick(float screenX, float screenY) {
	sf::Vector2f mouseWorldPos = window->mapPixelToCoords(sf::Vector2i(screenX, screenY));
	// Not sure why this is needed; probably something to do with RenderSystem coordinates being y-inversed
	mouseWorldPos.y = -mouseWorldPos.y;

	if (placingNewEnemy) {
		setPlacingNewEnemy(false);
		std::shared_ptr<EnemyEntityPlaceholder> enemy = std::make_shared<EnemyEntityPlaceholder>(nextPlaceholderID, registry, *spriteLoader, *levelPack, *queue);
		nextPlaceholderID++;
		undoStack.execute(UndoableCommand(
			[this, mouseWorldPos, enemy]() {
			mostRecentNewEnemyPlaceholderID = enemy->getID();
			enemy->moveTo(mouseWorldPos.x, mouseWorldPos.y);
			enemy->spawnVisualEntity();
			enemyPlaceholders[enemy->getID()] = enemy;
			updateEntityPlaceholdersList();
		},
			[this, enemy]() {
			deletePlaceholder(enemy->getID());
		}));
	} else if (manuallySettingPlaceholderPosition) {
		setManuallySettingPlaceholderPosition(selectedPlaceholder, false);
		float oldX = selectedPlaceholder->getX();
		float oldY = selectedPlaceholder->getY();
		undoStack.execute(UndoableCommand(
			[this, &selectedPlaceholder = this->selectedPlaceholder, mouseWorldPos]() {
			selectedPlaceholder->moveTo(mouseWorldPos.x, mouseWorldPos.y);
			if (selectedPlaceholder == this->selectedPlaceholder) {
				entityPlaceholderX->setValue(selectedPlaceholder->getX());
				entityPlaceholderY->setValue(selectedPlaceholder->getY());
			}
		},
			[this, &selectedPlaceholder = this->selectedPlaceholder, oldX, oldY]() {
			selectedPlaceholder->moveTo(oldX, oldY);
			if (selectedPlaceholder == this->selectedPlaceholder) {
				entityPlaceholderX->setValue(selectedPlaceholder->getX());
				entityPlaceholderY->setValue(selectedPlaceholder->getY());
			}
		}));
	} else if (playerPlaceholder->wasClicked(mouseWorldPos.x, mouseWorldPos.y)) {
		if (selectedPlaceholder == playerPlaceholder) {
			draggingPlaceholder = true;
			previousPlaceholderDragCoordsX = screenX;
			previousPlaceholderDragCoordsY = screenY;
			placeholderPosBeforeDragging.x = selectedPlaceholder->getX();
			placeholderPosBeforeDragging.y = selectedPlaceholder->getY();
		} else {
			selectPlaceholder(playerPlaceholder);
			justSelectedPlaceholder = true;
		}
	} else {
		for (auto p : enemyPlaceholders) {
			if (p.second->wasClicked(mouseWorldPos.x, mouseWorldPos.y)) {
				if (selectedPlaceholder == p.second) {
					draggingPlaceholder = true;
					previousPlaceholderDragCoordsX = screenX;
					previousPlaceholderDragCoordsY = screenY;
					placeholderPosBeforeDragging.x = selectedPlaceholder->getX();
					placeholderPosBeforeDragging.y = selectedPlaceholder->getY();
				} else {
					selectPlaceholder(p.second);
					justSelectedPlaceholder = true;
				}
				return;
			}
		}

		initialMousePressX = screenX;
		initialMousePressY = screenY;
	}
}

void GameplayTestWindow::runGameplayTest() {
	std::string message;
	bool good = true;
	for (auto p : enemyPlaceholders) {
		good = good && p.second->legalCheck(message, *levelPack, *spriteLoader);
	}
	clearLogs();
	logMessage(message);

	if (good) {
		testInProgress = true;
		startAndEndTest->setText("End test");
		playerPlaceholder->runTest();
		for (auto p : enemyPlaceholders) {
			p.second->runTest();
		}
	}
}

void GameplayTestWindow::endGameplayTest() {
	testInProgress = false;
	startAndEndTest->setText("Start test");

	playerPlaceholder->endTest();
	for (auto p : enemyPlaceholders) {
		p.second->endTest();
	}

	// Delete all entities except level manager
	uint32_t levelManager = registry.attachee<LevelManagerTag>();
	registry.each([&](uint32_t entity) {
		if (entity != levelManager) {
			registry.destroy(entity);
		}
	});
	// Respawn visual entities
	playerPlaceholder->spawnVisualEntity();
	for (auto p : enemyPlaceholders) {
		p.second->spawnVisualEntity();
	}

	levelPack->deleteTemporaryEditorObjecs();
}

void GameplayTestWindow::selectPlaceholder(std::shared_ptr<EntityPlaceholder> placeholder) {
	selectedPlaceholder = placeholder;
	
	ignoreSignal = true;
	rightPanel->setVisible(true);
	bool isEnemyPlaceholder = (dynamic_cast<EnemyEntityPlaceholder*>(placeholder.get()) != nullptr);
	selectedPlaceholderIsPlayer = !isEnemyPlaceholder;
	deleteEnemyPlaceholder->setEnabled(isEnemyPlaceholder);
	setEnemyPlaceholderTestMode->setVisible(isEnemyPlaceholder);
	testModeIDLabel->setVisible(isEnemyPlaceholder);
	testModeID->setVisible(isEnemyPlaceholder);
	entityPlaceholderX->setValue(placeholder->getX());
	entityPlaceholderY->setValue(placeholder->getY());
	if (isEnemyPlaceholder) {
		testModeID->removeAllItems();

		auto enemyPlaceholder = dynamic_cast<EnemyEntityPlaceholder*>(placeholder.get());
		if (enemyPlaceholder->getTestMode() == EnemyEntityPlaceholder::ENEMY) {
			testModeIDLabel->setText("Enemy ID");
			for (auto it = levelPack->getEnemyIteratorBegin(); it != levelPack->getEnemyIteratorEnd(); it++) {
				std::string ignoreThis; // useless var
				if (it->second->legal(ignoreThis)) {
					testModeID->addItem(it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
				}
			}
		} else if (enemyPlaceholder->getTestMode() == EnemyEntityPlaceholder::PHASE) {
			testModeIDLabel->setText("Phase ID");
			for (auto it = levelPack->getEnemyPhaseIteratorBegin(); it != levelPack->getEnemyPhaseIteratorEnd(); it++) {
				std::string ignoreThis; // useless var
				if (it->second->legal(ignoreThis)) {
					testModeID->addItem(it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
				}
			}
		} else if (enemyPlaceholder->getTestMode() == EnemyEntityPlaceholder::ATTACK_PATTERN) {
			testModeIDLabel->setText("Attack pattern ID");
			for (auto it = levelPack->getAttackPatternIteratorBegin(); it != levelPack->getAttackPatternIteratorEnd(); it++) {
				std::string ignoreThis; // useless var
				if (it->second->legal(ignoreThis)) {
					testModeID->addItem(it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
				}
			}
		} else if (enemyPlaceholder->getTestMode() == EnemyEntityPlaceholder::ATTACK) {
			testModeIDLabel->setText("Attack ID");
			for (auto it = levelPack->getAttackIteratorBegin(); it != levelPack->getAttackIteratorEnd(); it++) {
				std::string ignoreThis; // useless var
				if (it->second->legal(*levelPack, *spriteLoader, ignoreThis)) {
					testModeID->addItem(it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
				}
			}
		}

		testModeID->setSelectedItemById(std::to_string(static_cast<int>(enemyPlaceholder->getTestModeID())));
	}
	entityPlaceholdersList->setSelectedItemById(std::to_string(placeholder->getID()));
	ignoreSignal = false;
}

void GameplayTestWindow::deselectPlaceholder() {
	rightPanel->setVisible(false);
	currentCursor = nullptr;
	window->setMouseCursorVisible(true);
	selectedPlaceholder = nullptr;
	entityPlaceholdersList->deselectItem();
	deleteEnemyPlaceholder->setEnabled(false);
}

void GameplayTestWindow::deletePlaceholder(int placeholderID) {
	assert(placeholderID != playerPlaceholder->getID()); // Player placeholder can't be deleted

	if (selectedPlaceholder && placeholderID == selectedPlaceholder->getID()) {
		deselectPlaceholder();
	}

	enemyPlaceholders[placeholderID]->removePlaceholder();
	enemyPlaceholders.erase(placeholderID);
	entityPlaceholdersList->removeItemById(std::to_string(placeholderID));
}

void GameplayTestWindow::updateEntityPlaceholdersList() {
	entityPlaceholdersList->removeAllItems();
	entityPlaceholdersList->addItem("[id=" + std::to_string(playerPlaceholder->getID()) + "] Player", std::to_string(playerPlaceholder->getID()));
	for (auto p : enemyPlaceholders) {
		std::string text = "";
		if (!p.second->testModeIDSet()) {
			text = "Unset";
		} else {
			if (p.second->getTestMode() == EnemyEntityPlaceholder::ENEMY) {
				text = "Enemy";
			} else if (p.second->getTestMode() == EnemyEntityPlaceholder::PHASE) {
				text = "Phase";
			} else if (p.second->getTestMode() == EnemyEntityPlaceholder::ATTACK_PATTERN) {
				text = "A.Pattern";
			} else {
				text = "Attack";
			}
			text += " " + std::to_string(p.second->getTestModeID());
		}
		entityPlaceholdersList->addItem("[id=" + std::to_string(p.first) + "] " + text, std::to_string(p.first));
	}
}

void GameplayTestWindow::setPlacingNewEnemy(bool placingNewEnemy) {
	this->placingNewEnemy = placingNewEnemy;
	if (placingNewEnemy) {
		window->setMouseCursorVisible(false);
		currentCursor = movingEnemyPlaceholderCursor;
	} else if (!manuallySettingPlaceholderPosition) {
		window->setMouseCursorVisible(true);
		currentCursor = nullptr;
	}
}

void GameplayTestWindow::setManuallySettingPlaceholderPosition(std::shared_ptr<EntityPlaceholder> placeholder,  bool manuallySettingPlaceholderPosition) {
	this->manuallySettingPlaceholderPosition = manuallySettingPlaceholderPosition;
	if (manuallySettingPlaceholderPosition) {
		window->setMouseCursorVisible(false);

		bool isEnemyPlaceholder = (dynamic_cast<EnemyEntityPlaceholder*>(placeholder.get()) != nullptr);
		if (isEnemyPlaceholder) {
			currentCursor = movingEnemyPlaceholderCursor;
		} else {
			currentCursor = movingPlayerPlaceholderCursor;
		}
	} else if (!placingNewEnemy) {
		window->setMouseCursorVisible(true);
		currentCursor = nullptr;
	}
}

void GameplayTestWindow::toggleLogsDisplay() {
	if (bottomPanel->isVisible()) {
		bottomPanel->setVisible(false);
		toggleBottomPanelDisplay->setText("Show logs");
	} else {
		bottomPanel->setVisible(true);
		toggleBottomPanelDisplay->setText("Hide logs");
	}
}

void GameplayTestWindow::clearLogs() {
	logs->setText("");
}

void GameplayTestWindow::logMessage(std::string message) {
	logs->setText(logs->getText() + "\n" + message);
}

void GameplayTestWindow::EntityPlaceholder::moveTo(float x, float y) {
	this->x = x;
	this->y = y;
	if (registry.valid(visualEntity)) {
		auto& pos = registry.get<PositionComponent>(visualEntity);
		pos.setX(x);
		pos.setY(y);
	}
}

void GameplayTestWindow::EntityPlaceholder::endTest() {
	if (registry.valid(testEntity)) {
		registry.destroy(testEntity);
	}
}

bool GameplayTestWindow::EntityPlaceholder::wasClicked(int worldX, int worldY) {
	return std::sqrt((worldX - x)*(worldX - x) + (worldY - y)*(worldY - y)) <= CLICK_HITBOX_SIZE;
}

void GameplayTestWindow::EntityPlaceholder::removePlaceholder() {
	if (registry.valid(visualEntity)) {
		registry.destroy(visualEntity);
	}
	if (registry.valid(testEntity)) {
		registry.destroy(testEntity);
	}
}

void GameplayTestWindow::PlayerEntityPlaceholder::spawnVisualEntity() {
	assert(!registry.valid(visualEntity));
	visualEntity = registry.create();
	registry.assign<PositionComponent>(visualEntity, x, y);
	registry.assign<SpriteComponent>(visualEntity, spriteLoader, Animatable("Player Placeholder", "Default", true, LOCK_ROTATION), true, PLAYER_LAYER, 0);
}

void GameplayTestWindow::PlayerEntityPlaceholder::runTest() {
	registry.destroy(visualEntity);

	EditorPlayer params;
	std::string message; // does nothing
	if (!useDefaultTestPlayer && levelPack.getPlayer().legal(spriteLoader, message)) {
		params = levelPack.getPlayer();
	}
	else {
		// Create default player params
		auto playerAP = levelPack.createTempAttackPattern();
		auto playerAttack1 = levelPack.createTempAttack();
		auto pemp0 = playerAttack1->searchEMP(0);
		pemp0->setAnimatable(Animatable("Player Placeholder", "Default", true, LOCK_ROTATION));
		pemp0->setHitboxRadius(30);
		pemp0->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>(1, 0, 0));
		pemp0->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 700, 2), std::make_shared<ConstantTFV>(PI / 2.0f), 2.0f));
		pemp0->setOnCollisionAction(PIERCE_ENTITY);
		playerAP->addAttackID(0.1f, playerAttack1->getID());

		auto bombAP = levelPack.createTempAttackPattern();
		for (int i = 0; i < 10; i++) {
			auto bombAttack1 = levelPack.createTempAttack();
			auto b1emp0 = bombAttack1->searchEMP(0);
			b1emp0->setAnimatable(Animatable("Player Bullet", "Default", true, LOCK_ROTATION));
			b1emp0->setHitboxRadius(30);
			b1emp0->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>(1, 0, 0));
			b1emp0->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 1000, 2), std::make_shared<ConstantTFV>(1.0f + i * 0.13f), 2.0f));
			b1emp0->setOnCollisionAction(PIERCE_ENTITY);
			bombAP->addAttackID(0, bombAttack1->getID());
		}

		auto pset1 = EntityAnimatableSet(Animatable("Player Placeholder", "Default", true, ROTATE_WITH_MOVEMENT),
			Animatable("Player Placeholder", "Default", true, ROTATE_WITH_MOVEMENT),
			Animatable("Player Placeholder", "Default", true, ROTATE_WITH_MOVEMENT),
			std::make_shared<PlayAnimatableDeathAction>(Animatable("Player Placeholder", "Default", true, ROTATE_WITH_MOVEMENT), PlayAnimatableDeathAction::NONE, 3.0f));

		params = EditorPlayer(10, 11, 300, 100, 5, 0, 0, 2.0f, std::vector<PlayerPowerTier>{ PlayerPowerTier(pset1, playerAP->getID(), 0.1f, playerAP->getID(), 0.5f, bombAP->getID(), 5.0f) }, SoundSettings("oof.wav", 10), SoundSettings("death.ogg"), Animatable("heart.png", "", true, LOCK_ROTATION),
			3, 10, Animatable("bomb.png", "", true, LOCK_ROTATION), SoundSettings("bomb_ready.wav"), 5.0f);
	}

	testEntity = registry.create();
	registry.assign<AnimatableSetComponent>(testEntity);
	auto& playerTag = registry.assign<PlayerTag>(entt::tag_t{}, testEntity, registry, levelPack, testEntity, params.getSpeed(), params.getFocusedSpeed(), params.getInvulnerabilityTime(),
		params.getPowerTiers(), params.getHurtSound(), params.getDeathSound(), params.getInitialBombs(), params.getMaxBombs(), params.getBombInvincibilityTime());
	auto& health = registry.assign<HealthComponent>(testEntity, params.getInitialHealth(), params.getMaxHealth());
	// Hitbox temporarily at 0, 0 until an Animatable is assigned to the player later
	registry.assign<HitboxComponent>(testEntity, LOCK_ROTATION, params.getHitboxRadius(), 0, 0);
	registry.assign<PositionComponent>(testEntity, PLAYER_SPAWN_X - params.getHitboxPosX(), PLAYER_SPAWN_Y - params.getHitboxPosY());
	registry.assign<SpriteComponent>(testEntity, PLAYER_LAYER, 0);
}

void GameplayTestWindow::EnemyEntityPlaceholder::runTest() {
	registry.destroy(visualEntity);

	EnemySpawnInfo info;
	if (testMode == ENEMY) {
		info = EnemySpawnInfo(testModeID, x, y, std::vector<std::pair<std::shared_ptr<Item>, int>>());
	} else if (testMode == PHASE) {
		std::shared_ptr<EditorEnemy> enemy = levelPack.createTempEnemy();
		EntityAnimatableSet enemyAnimatableSet(Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT), Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT), Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT));
		enemy->setHitboxRadius(100);
		enemy->setHealth(2000000000);
		enemy->addPhaseID(0, std::make_shared<TimeBasedEnemyPhaseStartCondition>(0), testModeID, enemyAnimatableSet);
	} else if (testMode == ATTACK_PATTERN) {
		std::shared_ptr<EditorEnemyPhase> phase = levelPack.createTempEnemyPhase();
		phase->addAttackPatternID(0, testModeID);
		// TODO: make this customizable
		phase->setAttackPatternLoopDelay(2);

		std::shared_ptr<EditorEnemy> enemy = levelPack.createTempEnemy();
		EntityAnimatableSet enemyAnimatableSet(Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT), Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT), Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT));
		enemy->setHitboxRadius(100);
		enemy->setHealth(2000000000);
		enemy->addPhaseID(0, std::make_shared<TimeBasedEnemyPhaseStartCondition>(0), phase->getID(), enemyAnimatableSet);
	} else {
		std::shared_ptr<EditorAttackPattern> attackPattern = levelPack.createTempAttackPattern();
		attackPattern->addAttackID(0, testModeID);

		std::shared_ptr<EditorEnemyPhase> phase = levelPack.createTempEnemyPhase();
		phase->addAttackPatternID(0, attackPattern->getID());
		// TODO: make this customizable
		phase->setAttackPatternLoopDelay(2);

		std::shared_ptr<EditorEnemy> enemy = levelPack.createTempEnemy();
		EntityAnimatableSet enemyAnimatableSet(Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT), Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT), Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT));
		enemy->setHitboxRadius(100);
		enemy->setHealth(2000000000);
		enemy->addPhaseID(0, std::make_shared<TimeBasedEnemyPhaseStartCondition>(0), phase->getID(), enemyAnimatableSet);
	}
	info.spawnEnemy(spriteLoader, levelPack, registry, queue);
}

void GameplayTestWindow::EnemyEntityPlaceholder::spawnVisualEntity() {
	assert(!registry.valid(visualEntity));
	visualEntity = registry.create();
	registry.assign<PositionComponent>(visualEntity, x, y);
	registry.assign<SpriteComponent>(visualEntity, spriteLoader, Animatable("Enemy Placeholder", "Default", true, LOCK_ROTATION), true, PLAYER_LAYER, 0);
}

bool GameplayTestWindow::EnemyEntityPlaceholder::legalCheck(std::string & message, LevelPack & levelPack, SpriteLoader & spriteLoader) {
	bool good = true;
	if (!testModeIDHasBeenSet) {
		message += "[id=" + std::to_string(id) + "] Test mode ID has not been set.\n";
		good = false;
	} else {
		if (testMode == ENEMY) {
			if (!levelPack.hasEnemy(testModeID)) {
				message += "[id=" + std::to_string(id) + "] Enemy ID " + std::to_string(testModeID) + " no longer exists.\n";
				good = false;
			} else {
				good = good && levelPack.getEnemy(testModeID)->legal(message);
			}
		} else if (testMode == PHASE) {
			if (!levelPack.hasEnemyPhase(testModeID)) {
				message += "[id=" + std::to_string(id) + "] Enemy phase ID " + std::to_string(testModeID) + " no longer exists.\n";
				good = false;
			} else {
				good = good && levelPack.getEnemyPhase(testModeID)->legal(message);
			}
		} else if (testMode == ATTACK_PATTERN) {
			if (!levelPack.hasAttackPattern(testModeID)) {
				message += "[id=" + std::to_string(id) + "] Attack pattern ID " + std::to_string(testModeID) + " no longer exists.\n";
				good = false;
			} else {
				good = good && levelPack.getAttackPattern(testModeID)->legal(message);
			}
		} else {
			if (!levelPack.hasAttack(testModeID)) {
				message += "[id=" + std::to_string(id) + "] Attack pattern ID " + std::to_string(testModeID) + " no longer exists.\n";
				good = false;
			} else {
				good = good && levelPack.getAttack(testModeID)->legal(levelPack, spriteLoader, message);
			}
		}
	}
	return good;
}
