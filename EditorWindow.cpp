#include "EditorWindow.h"
#include "Components.h"
#include <algorithm>
#include <boost/date_time.hpp>
#include <sstream>
#include <iterator>

EditorWindow::EditorWindow(std::shared_ptr<std::mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize, bool letterboxingEnabled, float renderInterval) :
	tguiMutex(tguiMutex), windowTitle(windowTitle), windowWidth(width), windowHeight(height), scaleWidgetsOnResize(scaleWidgetsOnResize), letterboxingEnabled(letterboxingEnabled), renderInterval(renderInterval) {
	std::lock_guard<std::mutex> lock(*tguiMutex);
	gui = std::make_shared<tgui::Gui>();
	closeSignal = std::make_shared<entt::SigH<void()>>();

	confirmationPanel = tgui::Panel::create();
	confirmationText = tgui::Label::create();
	confirmationYes = tgui::Button::create();
	confirmationNo = tgui::Button::create();
	confirmationText->setTextSize(TEXT_SIZE);
	confirmationYes->setTextSize(TEXT_SIZE);
	confirmationNo->setTextSize(TEXT_SIZE);
	confirmationText->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	confirmationYes->setText("Yes");
	confirmationNo->setText("No");
	confirmationYes->setSize(100.0f, TEXT_BUTTON_HEIGHT);
	confirmationNo->setSize(100.0f, TEXT_BUTTON_HEIGHT);

	confirmationYes->connect("Pressed", [&]() {
		assert(confirmationSignal); // This button cannot be pressed unless promptConfirmation() was called first
		confirmationSignal->publish(true);
		closeConfirmationPanel();
	});
	confirmationNo->connect("Pressed", [&]() {
		assert(confirmationSignal); // This button cannot be pressed unless promptConfirmation() was called first
		confirmationSignal->publish(false);
		closeConfirmationPanel();
	});

	confirmationPanel->add(confirmationText);
	confirmationPanel->add(confirmationYes);
	confirmationPanel->add(confirmationNo);
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
					closeSignal->publish();
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

std::shared_ptr<entt::SigH<void(bool)>> EditorWindow::promptConfirmation(std::string message) {
	confirmationSignal = std::make_shared<entt::SigH<void(bool)>>();

	// Disable all widgets
	widgetsToBeEnabledAfterConfirmationPrompt.clear();
	for (std::shared_ptr<tgui::Widget> ptr : gui->getWidgets()) {
		if (ptr->isEnabled()) {
			widgetsToBeEnabledAfterConfirmationPrompt.push_back(ptr);
		}
		ptr->setEnabled(false);
	}

	confirmationText->setText(message);
	gui->add(confirmationPanel);

	return confirmationSignal;
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

	confirmationPanel->setSize(std::max(300.0f, windowWidth * 0.35f), std::max(200.0f, windowHeight * 0.25f));
	confirmationPanel->setPosition(windowWidth / 2.0f - confirmationPanel->getSize().x / 2.0f, windowHeight / 2.0f - confirmationPanel->getSize().y / 2.0f);
	confirmationText->setMaximumTextWidth(confirmationPanel->getSize().x - GUI_PADDING_X * 2);
	confirmationYes->setPosition(confirmationPanel->getSize().x/2.0f - confirmationYes->getSize().x - GUI_PADDING_X/2.0f, confirmationPanel->getSize().y - confirmationYes->getSize().y - GUI_PADDING_Y);
	confirmationNo->setPosition(tgui::bindRight(confirmationYes) + GUI_PADDING_X, tgui::bindTop(confirmationYes));

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

std::shared_ptr<entt::SigH<void()>> EditorWindow::getCloseSignal() {
	return closeSignal;
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

void EditorWindow::closeConfirmationPanel() {
	gui->remove(confirmationPanel);

	for (auto ptr : widgetsToBeEnabledAfterConfirmationPrompt) {
		ptr->setEnabled(true);
	}
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
	registryMutex = std::make_shared<std::mutex>();

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

	onBezierControlPointEditingEnd = std::make_shared<entt::SigH<void(bool, std::vector<sf::Vector2f>)>>();

	leftPanel = tgui::ScrollablePanel::create();
	entityPlaceholdersListLabel = tgui::Label::create();
	entityPlaceholdersList = std::make_shared<ScrollableListBox>();
	newEnemyPlaceholder = tgui::Button::create();
	deleteEnemyPlaceholder = tgui::Button::create();
	startAndEndTest = tgui::Button::create();
	toggleBottomPanelDisplay = tgui::Button::create();
	addControlPointPlaceholderAbove = tgui::Button::create();
	addControlPointPlaceholderBelow = tgui::Button::create();

	rightPanel = tgui::ScrollablePanel::create();
	entityPlaceholderXLabel = tgui::Label::create();
	entityPlaceholderX = std::make_shared<NumericalEditBoxWithLimits>();
	entityPlaceholderYLabel = tgui::Label::create();
	entityPlaceholderY = std::make_shared<NumericalEditBoxWithLimits>();
	entityPlaceholderManualSet = tgui::Button::create();
	setEnemyPlaceholderTestMode = tgui::Button::create();
	testModeIDLabel = tgui::Label::create();
	testModeID = std::make_shared<ScrollableListBox>();
	testModePopup = std::make_shared<ScrollableListBox>();
	showMovementPathLabel = tgui::Label::create();
	showMovementPath = tgui::CheckBox::create();

	bottomPanel = tgui::ScrollablePanel::create();
	logs = tgui::Label::create();

	externalEndTest = tgui::Button::create();
	bezierFinishEditing = tgui::Button::create();

	entityPlaceholdersList->getListBox()->setAutoScroll(false);
	testModeID->getListBox()->setAutoScroll(false);
	externalEndTest->setVisible(false);
	bezierFinishEditing->setVisible(false);

	entityPlaceholdersListLabel->setText("Entities");
	newEnemyPlaceholder->setText("New enemy");
	deleteEnemyPlaceholder->setText("Delete enemy");
	entityPlaceholderXLabel->setText("X");
	entityPlaceholderYLabel->setText("Y");
	entityPlaceholderManualSet->setText("Manual set");
	setEnemyPlaceholderTestMode->setText("Set test mode");
	startAndEndTest->setText("Start test");
	toggleBottomPanelDisplay->setText("Show logs");
	externalEndTest->setText("End test");
	bezierFinishEditing->setText("Finish editing");
	showMovementPathLabel->setText("Show movement path");

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
	externalEndTest->setTextSize(TEXT_SIZE);
	addControlPointPlaceholderAbove->setTextSize(TEXT_SIZE);
	addControlPointPlaceholderBelow->setTextSize(TEXT_SIZE);
	bezierFinishEditing->setTextSize(TEXT_SIZE);
	showMovementPathLabel->setTextSize(TEXT_SIZE);

	testModePopup->getListBox()->addItem("Enemy", std::to_string(static_cast<int>(EnemyEntityPlaceholder::ENEMY)));
	testModePopup->getListBox()->addItem("Enemy phase", std::to_string(static_cast<int>(EnemyEntityPlaceholder::PHASE)));
	testModePopup->getListBox()->addItem("Attack pattern", std::to_string(static_cast<int>(EnemyEntityPlaceholder::ATTACK_PATTERN)));
	testModePopup->getListBox()->addItem("Attack", std::to_string(static_cast<int>(EnemyEntityPlaceholder::ATTACK)));
	testModePopup->getListBox()->setItemHeight(20);
	testModePopup->setSize(150, testModePopup->getListBox()->getItemHeight() * testModePopup->getListBox()->getItemCount());
	testModePopup->setPosition(tgui::bindLeft(setEnemyPlaceholderTestMode), tgui::bindTop(setEnemyPlaceholderTestMode));

	showMovementPath->connect("Changed", [&]() {
		if (ignoreSignal) return;
		if (showMovementPath->isChecked()) {
			showPlaceholderMovementPath(selectedPlaceholder);
		} else if (placeholderMovementPaths.count(selectedPlaceholder->getID()) > 0) {
			placeholderMovementPaths.erase(selectedPlaceholder->getID());
		}
	});
	entityPlaceholdersList->getListBox()->connect("ItemSelected", [&](std::string item, std::string id) {
		if (ignoreSignal) return;
		if (id == "") {
			deselectPlaceholder();
		} else {
			if (std::stoi(id) == playerPlaceholder->getID()) {
				selectPlaceholder(playerPlaceholder);
			} else {
				selectPlaceholder(nonplayerPlaceholders[std::stoi(id)]);
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
			nonplayerPlaceholders[id] = enemy;
			updateEntityPlaceholdersList();
		}));
	});
	entityPlaceholderManualSet->connect("Pressed", [&]() {
		setPlacingNewEnemy(false);
		setManuallySettingPlaceholderPosition(selectedPlaceholder, true);
	});
	entityPlaceholderX->getOnValueSet()->sink().connect<GameplayTestWindow, &GameplayTestWindow::onEntityPlaceholderXValueSet>(this);
	entityPlaceholderY->getOnValueSet()->sink().connect<GameplayTestWindow, &GameplayTestWindow::onEntityPlaceholderYValueSet>(this);
	testModeID->getListBox()->connect("ItemSelected", [&](std::string itemName, std::string itemID) {
		if (itemID != "") {
			assert(dynamic_cast<EnemyEntityPlaceholder*>(selectedPlaceholder.get()) != nullptr); // Can only change test mode ID of EnemyEntityPlaceholder
			auto enemyPlaceholder = dynamic_cast<EnemyEntityPlaceholder*>(selectedPlaceholder.get());
			enemyPlaceholder->setTestModeID(std::stoi(itemID));
			updateEntityPlaceholdersList();
		}
	});
	setEnemyPlaceholderTestMode->connect("Pressed", [&]() {
		addPopupWidget(rightPanel, testModePopup);
	});
	testModePopup->getListBox()->connect("MousePressed", [&](std::string itemName, std::string itemID) {
		if (ignoreSignal) return;

		assert(dynamic_cast<EnemyEntityPlaceholder*>(selectedPlaceholder.get()) != nullptr); // Can only change test mode of EnemyEntityPlaceholder
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
	externalEndTest->connect("Pressed", [&]() {
		assert(testInProgress);
		endGameplayTest();
	});
	addControlPointPlaceholderAbove->connect("Pressed", [&]() {
		if (selectedPlaceholder) {
			nextBezierControlPointPlaceholderDesiredID = selectedPlaceholder->getID();
		} else {
			nextBezierControlPointPlaceholderDesiredID = 0;
		}
		setPlacingNewEnemy(true);
		setManuallySettingPlaceholderPosition(selectedPlaceholder, false);
	});
	addControlPointPlaceholderBelow->connect("Pressed", [&]() {
		if (selectedPlaceholder) {
			nextBezierControlPointPlaceholderDesiredID = selectedPlaceholder->getID() + 1;
		} else {
			if (nonplayerPlaceholders.size() == 0) {
				nextBezierControlPointPlaceholderDesiredID = 0;
			} else {
				nextBezierControlPointPlaceholderDesiredID = nonplayerPlaceholders.rbegin()->first + 1;
			}
		}
		setPlacingNewEnemy(true);
		setManuallySettingPlaceholderPosition(selectedPlaceholder, false);
	});
	bezierFinishEditing->connect("Pressed", [&]() {
		if (nonplayerPlaceholders.size() <= 1) {
			logMessage("Number of control points must be >= 2");
			if (!bottomPanel->isVisible()) {
				toggleLogsDisplay();
			}
			return;
		}

		// Prompt user for whether to save changes
		auto saveChangesSignal = promptConfirmation("Save changes made to the control points?");
		saveChangesSignal->sink().connect<GameplayTestWindow, &GameplayTestWindow::onBezierFinishEditingConfirmationPrompt>(this);
	});

	addControlPointPlaceholderAbove->setVisible(false);
	addControlPointPlaceholderBelow->setVisible(false);
	bezierFinishEditing->setVisible(false);

	leftPanel->add(entityPlaceholdersListLabel);
	leftPanel->add(entityPlaceholdersList);
	leftPanel->add(newEnemyPlaceholder);
	leftPanel->add(deleteEnemyPlaceholder);
	leftPanel->add(startAndEndTest);
	leftPanel->add(toggleBottomPanelDisplay);
	leftPanel->add(addControlPointPlaceholderAbove);
	leftPanel->add(addControlPointPlaceholderBelow);
	rightPanel->add(entityPlaceholderXLabel);
	rightPanel->add(entityPlaceholderX);
	rightPanel->add(entityPlaceholderYLabel);
	rightPanel->add(entityPlaceholderY);
	rightPanel->add(entityPlaceholderManualSet);
	rightPanel->add(setEnemyPlaceholderTestMode);
	rightPanel->add(testModeIDLabel);
	rightPanel->add(testModeID);
	rightPanel->add(showMovementPathLabel);
	rightPanel->add(showMovementPath);
	bottomPanel->add(logs);

	getGui()->add(leftPanel);
	getGui()->add(bezierFinishEditing);
	getGui()->add(rightPanel);
	getGui()->add(bottomPanel);
	getGui()->add(externalEndTest);

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

	levelPack->getOnChange()->sink().connect<GameplayTestWindow, &GameplayTestWindow::onLevelPackChange>(this);
}

void GameplayTestWindow::addEMPTestPlaceholder(std::shared_ptr<EditorMovablePoint> emp, bool empIsFromAttack, int sourceID) {
	std::shared_ptr<EMPTestEntityPlaceholder> empPlaceholder = std::make_shared<EMPTestEntityPlaceholder>(nextPlaceholderID++, registry, *spriteLoader, *levelPack, *queue, emp, sourceID, empIsFromAttack);
	empPlaceholder->spawnVisualEntity();
	nonplayerPlaceholders[empPlaceholder->getID()] = empPlaceholder;
	updateEntityPlaceholdersList();

	selectPlaceholder(empPlaceholder);
	setPlacingNewEnemy(false);
	setManuallySettingPlaceholderPosition(selectedPlaceholder, true);
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
			setEntityPlaceholderXWidgetValue(selectedPlaceholder->getX());
			setEntityPlaceholderYWidgetValue(selectedPlaceholder->getY());
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
						if (placeholderMovementPaths.count(selectedPlaceholder->getID()) > 0) {
							// Update movement path
							showPlaceholderMovementPath(selectedPlaceholder);
						}
						if (selectedPlaceholder == this->selectedPlaceholder) {
							setEntityPlaceholderXWidgetValue(selectedPlaceholder->getX());
							setEntityPlaceholderYWidgetValue(selectedPlaceholder->getY());
						}
						if (editingBezierControlPoints) {
							// Update coordinates in placeholders list
							updateEntityPlaceholdersList();
						}
					},
						[this, &selectedPlaceholder = this->selectedPlaceholder, &placeholderPosBeforeDragging = this->placeholderPosBeforeDragging]() {
						selectedPlaceholder->moveTo(placeholderPosBeforeDragging.x, placeholderPosBeforeDragging.y);
						if (placeholderMovementPaths.count(selectedPlaceholder->getID()) > 0) {
							// Update movement path
							showPlaceholderMovementPath(selectedPlaceholder);
						}
						if (selectedPlaceholder == this->selectedPlaceholder) {
							setEntityPlaceholderXWidgetValue(selectedPlaceholder->getX());
							setEntityPlaceholderYWidgetValue(selectedPlaceholder->getY());
						}
						if (editingBezierControlPoints) {
							// Update coordinates in placeholders list
							updateEntityPlaceholdersList();
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

		if (testInProgress) {
			try {
				std::lock_guard<std::mutex> lock(*registryMutex);

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
			} catch (...) {
				endGameplayTest();
			}
		}
	}
}

void GameplayTestWindow::render(float deltaTime) {
	if (!testInProgress) {
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
	}

	registryMutex->lock();
	if (!paused) {
		spriteAnimationSystem->update(deltaTime);
	}
	debugRenderSystem->update(deltaTime);
	// Render movement paths
	if (!testInProgress) {
		if (editingBezierControlPoints) {
			window->draw(bezierMovementPath);
		} else {
			for (auto it = placeholderMovementPaths.begin(); it != placeholderMovementPaths.end(); it++) {
				window->draw(it->second);
			}
		}
	}
	registryMutex->unlock();

	UndoableEditorWindow::render(deltaTime);

	if (currentCursor) {
		sf::View oldView = window->getView();
		sf::View fixedView = sf::View(sf::FloatRect(0, -(float)window->getSize().y, window->getSize().x, window->getSize().y));
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
	addControlPointPlaceholderAbove->setPosition(GUI_PADDING_X, tgui::bindBottom(entityPlaceholdersList) + GUI_PADDING_Y);
	addControlPointPlaceholderBelow->setPosition(GUI_PADDING_X, tgui::bindBottom(addControlPointPlaceholderAbove) + GUI_PADDING_Y);
	newEnemyPlaceholder->setPosition(GUI_PADDING_X, tgui::bindBottom(entityPlaceholdersList) + GUI_PADDING_Y);
	if (newEnemyPlaceholder->isVisible()) {
		deleteEnemyPlaceholder->setPosition(GUI_PADDING_X, tgui::bindBottom(newEnemyPlaceholder) + GUI_PADDING_Y);
	} else {
		deleteEnemyPlaceholder->setPosition(GUI_PADDING_X, tgui::bindBottom(addControlPointPlaceholderBelow) + GUI_PADDING_Y);
	}
	startAndEndTest->setPosition(GUI_PADDING_X, tgui::bindBottom(deleteEnemyPlaceholder) + GUI_PADDING_Y * 2);
	toggleBottomPanelDisplay->setPosition(GUI_PADDING_X, tgui::bindBottom(startAndEndTest) + GUI_PADDING_Y);
	entityPlaceholdersList->setSize(leftPanelWidth - GUI_PADDING_X * 2, std::max(height * 0.75f, entityPlaceholdersList->getListBox()->getItemHeight() * 5.0f));
	entityPlaceholdersList->onResize();
	newEnemyPlaceholder->setSize(std::max(leftPanelWidth - GUI_PADDING_X * 2, 100.0f), TEXT_BUTTON_HEIGHT);
	addControlPointPlaceholderAbove->setSize(std::max(leftPanelWidth - GUI_PADDING_X * 2, 100.0f), TEXT_BUTTON_HEIGHT);
	addControlPointPlaceholderBelow->setSize(std::max(leftPanelWidth - GUI_PADDING_X * 2, 100.0f), TEXT_BUTTON_HEIGHT);
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
	showMovementPathLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(testModeID) + GUI_PADDING_Y);
	showMovementPath->setPosition(GUI_PADDING_X, tgui::bindBottom(showMovementPathLabel) + GUI_PADDING_Y);
	entityPlaceholderX->setSize(rightPanelWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	entityPlaceholderY->setSize(rightPanelWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	entityPlaceholderManualSet->setSize(std::max(rightPanelWidth - GUI_PADDING_X * 2, 100.0f), TEXT_BUTTON_HEIGHT);
	setEnemyPlaceholderTestMode->setSize(std::max(rightPanelWidth - GUI_PADDING_X * 2, 100.0f), TEXT_BUTTON_HEIGHT);
	testModeID->setSize(rightPanelWidth - GUI_PADDING_X * 2, height * 0.5f);
	testModeID->onResize();
	showMovementPath->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);

	const float bottomPanelWidth = width - leftPanelWidth - rightPanelWidth;
	const float bottomPanelHeight = height * 0.25f;
	bottomPanel->setSize(bottomPanelWidth, bottomPanelHeight);
	bottomPanel->setPosition(tgui::bindRight(leftPanel), height - bottomPanelHeight);
	logs->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	logs->setSize(bottomPanelWidth - GUI_PADDING_X * 2, bottomPanelHeight - GUI_PADDING_Y);
	logs->setMaximumTextWidth(logs->getSize().x);

	externalEndTest->setSize(100.0f, TEXT_BOX_HEIGHT);
	externalEndTest->setPosition(0, height - externalEndTest->getSize().y);
	bezierFinishEditing->setSize(100.0f, TEXT_BOX_HEIGHT);
	bezierFinishEditing->setPosition(width - bezierFinishEditing->getSize().x, height - bezierFinishEditing->getSize().y);

    setCameraZoom(cameraZoom);
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

	debugRenderSystem = std::make_unique<DebugRenderSystem>(registry, *window, MAP_WIDTH, MAP_HEIGHT);

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
	debugRenderSystem->setBackground(std::move(background));
	debugRenderSystem->setBackgroundScrollSpeedX(level->getBackgroundScrollSpeedX());
	debugRenderSystem->setBackgroundScrollSpeedY(level->getBackgroundScrollSpeedY());

	deselectPlaceholder();
}

void GameplayTestWindow::beginEditingBezierControlPoints(MoveCustomBezierEMPA* bezierEMPA) {
	if (testInProgress) {
		endGameplayTest();
	}

	editingBezierControlPoints = true;
	editingBezierEMPALifespan = bezierEMPA->getTime();

	newEnemyPlaceholder->setVisible(false);
	addControlPointPlaceholderAbove->setVisible(true);
	addControlPointPlaceholderBelow->setVisible(true);
	bezierFinishEditing->setVisible(true);
	deleteEnemyPlaceholder->setText("Delete control point");
	updateWindowView(window->getSize().x, window->getSize().y);

	// Cache existing placeholders
	playerPlaceholder->removePlaceholder(registryMutex);
	for (auto p : nonplayerPlaceholders) {
		p.second->removePlaceholder(registryMutex);
	}
	cachedNonplayerPlaceholdersForEditingBezierControlPoints = nonplayerPlaceholders;
	nonplayerPlaceholders.clear();
	// Create placeholder for existing control points
	int i = 0;
	float xOffset = MAP_WIDTH/2.0f, yOffset = MAP_HEIGHT/2.0f;
	for (auto cp : bezierEMPA->getUnrotatedControlPoints()) {
		insertBezierControlPointPlaceholder(i++, cp.x + xOffset, cp.y + yOffset);
	}

	showBezierMovementPath();

	deselectPlaceholder();
	updateEntityPlaceholdersList();
}

void GameplayTestWindow::endEditingBezierControlPoints(bool saveChanges, std::vector<sf::Vector2f> newControlPoints) {
	if (testInProgress) {
		endGameplayTest();
	}

	editingBezierControlPoints = false;
	onBezierControlPointEditingEnd->publish(saveChanges, newControlPoints);

	newEnemyPlaceholder->setVisible(true);
	addControlPointPlaceholderAbove->setVisible(false);
	addControlPointPlaceholderBelow->setVisible(false);
	bezierFinishEditing->setVisible(false);
	deleteEnemyPlaceholder->setText("Delete enemy");
	updateWindowView(window->getSize().x, window->getSize().y);

	// Restore placeholders from cache
	playerPlaceholder->removePlaceholder(registryMutex);
	for (auto p : nonplayerPlaceholders) {
		p.second->removePlaceholder(registryMutex);
	}
	nonplayerPlaceholders = cachedNonplayerPlaceholdersForEditingBezierControlPoints;
	playerPlaceholder->spawnVisualEntity();
	for (auto p : nonplayerPlaceholders) {
		p.second->spawnVisualEntity();
	}

	deselectPlaceholder();
	updateEntityPlaceholdersList();
}

void GameplayTestWindow::insertBezierControlPointPlaceholder(int id, float x, float y) {
	std::shared_ptr<BezierControlPointPlaceholder> cp = std::make_shared<BezierControlPointPlaceholder>(id, registry, *spriteLoader, *levelPack);
	cp->moveTo(x, y);
	mostRecentNewEnemyPlaceholderID = id;
	cp->spawnVisualEntity();

	undoStack.execute(UndoableCommand(
		[this, cp, id]() {
		if (nonplayerPlaceholders.count(id) == 0) {
			// There is room for the new placeholder, so just insert it into the map
			nonplayerPlaceholders[id] = cp;

			// Update nextPlaceholderID so that it remains at least 1 greater than the current greatest ID
			if (id > nextPlaceholderID) {
				nextPlaceholderID = id + 1;
			}
		} else {
			// There is no room for the new placeholder, so increment all placeholders' IDs that are greater than id to make room

			// Loop until nextPlaceholderID because nextPlaceholderID is always at least 1 greater than the current greatest ID
			std::shared_ptr<EntityPlaceholder> temp = cp;
			std::shared_ptr<EntityPlaceholder> temp2;
			int i;
			for (i = id; i < nextPlaceholderID + 1; i++) {
				if (nonplayerPlaceholders.count(i) > 0) {
					temp2 = nonplayerPlaceholders[i];
					nonplayerPlaceholders[i] = temp;
					nonplayerPlaceholders[i]->setID(i);
					temp = temp2;
				} else {
					nonplayerPlaceholders[i] = temp;
					nonplayerPlaceholders[i]->setID(i);
					break;
				}
			}
			// Update nextPlaceholderID so that it remains at least 1 greater than the current greatest ID
			if (i == nextPlaceholderID) {
				nextPlaceholderID = i + 1;
			}
		}
		selectPlaceholder(cp);
		updateEntityPlaceholdersList();
	},
		[this, cp]() {
		deletePlaceholder(cp->getID());
	}));
}

void GameplayTestWindow::onBezierFinishEditingConfirmationPrompt(bool saveChanges) {
	assert(nonplayerPlaceholders.size() >= 2);

	if (saveChanges) {
		// Convert existing placeholders to control points, relative to the first control point
		std::vector<sf::Vector2f> cps;
		sf::Vector2f offset = sf::Vector2f(nonplayerPlaceholders[0]->getX(), nonplayerPlaceholders[0]->getY());
		for (auto p : nonplayerPlaceholders) {
			cps.push_back(sf::Vector2f(p.second->getX(), p.second->getY()) - offset);
		}
		endEditingBezierControlPoints(true, cps);
	} else {
		endEditingBezierControlPoints(false, std::vector<sf::Vector2f>());
	}
}

void GameplayTestWindow::setEntityPlaceholderXWidgetValue(float value) {
	if (editingBezierControlPoints) {
		// Value is relative to the position of the first bezier control point
		if (nonplayerPlaceholders.size() == 0) {
			entityPlaceholderX->setValue(0);
		} else {
			entityPlaceholderX->setValue(value - nonplayerPlaceholders[0]->getX());
		}
	} else {
		entityPlaceholderX->setValue(value);
	}
}

void GameplayTestWindow::setEntityPlaceholderYWidgetValue(float value) {
	if (editingBezierControlPoints) {
		// Value is relative to the position of the first bezier control point
		if (nonplayerPlaceholders.size() == 0) {
			entityPlaceholderY->setValue(0);
		} else {
			entityPlaceholderY->setValue(value - nonplayerPlaceholders[0]->getY());
		}
	} else {
		entityPlaceholderY->setValue(value);
	}
}

void GameplayTestWindow::onEntityPlaceholderXValueSet(float value) {
	float oldValue = selectedPlaceholder->getX();
	undoStack.execute(UndoableCommand(
		[this, &selectedPlaceholder = this->selectedPlaceholder, value]() {
		selectedPlaceholder->moveTo(value, selectedPlaceholder->getY());
		if (placeholderMovementPaths.count(selectedPlaceholder->getID()) > 0) {
			// Update movement path
			showPlaceholderMovementPath(selectedPlaceholder);
		}
		if (editingBezierControlPoints) {
			// Update entity placeholder list because control point entries have their coordinates
			updateEntityPlaceholdersList();
		}
	},
		[this, &selectedPlaceholder = this->selectedPlaceholder, oldValue]() {
		selectedPlaceholder->moveTo(oldValue, selectedPlaceholder->getY());
		if (placeholderMovementPaths.count(selectedPlaceholder->getID()) > 0) {
			// Update movement path
			showPlaceholderMovementPath(selectedPlaceholder);
		}
		if (editingBezierControlPoints) {
			// Update entity placeholder list because control point entries have their coordinates
			updateEntityPlaceholdersList();
		}
	}));
}

void GameplayTestWindow::onEntityPlaceholderYValueSet(float value) {
	float oldValue = selectedPlaceholder->getY();
	undoStack.execute(UndoableCommand(
		[this, &selectedPlaceholder = this->selectedPlaceholder, value]() {
		selectedPlaceholder->moveTo(selectedPlaceholder->getX(), value);
		if (placeholderMovementPaths.count(selectedPlaceholder->getID()) > 0) {
			// Update movement path
			showPlaceholderMovementPath(selectedPlaceholder);
		}
		if (editingBezierControlPoints) {
			// Update coordinates in placeholders list
			updateEntityPlaceholdersList();
		}
	},
		[this, &selectedPlaceholder = this->selectedPlaceholder, oldValue]() {
		selectedPlaceholder->moveTo(selectedPlaceholder->getX(), oldValue);
		if (placeholderMovementPaths.count(selectedPlaceholder->getID()) > 0) {
			// Update movement path
			showPlaceholderMovementPath(selectedPlaceholder);
		}
		if (editingBezierControlPoints) {
			// Update coordinates in placeholders list
			updateEntityPlaceholdersList();
		}
	}));
}

void GameplayTestWindow::onLevelPackChange() {
	updateEntityPlaceholdersList();
	deselectPlaceholder();

	if (testInProgress) {
		endGameplayTest();
	}
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
	view.setSize(window->getSize().x / zoom, window->getSize().y / zoom);
	window->setView(view);
}

void GameplayTestWindow::onGameplayAreaMouseClick(float screenX, float screenY) {
	sf::Vector2f mouseWorldPos = window->mapPixelToCoords(sf::Vector2i(screenX, screenY));
	// Not sure why this is needed; probably something to do with RenderSystem coordinates being y-inversed
	mouseWorldPos.y = -mouseWorldPos.y;

	if (placingNewEnemy) {
		if (editingBezierControlPoints) {
			setPlacingNewEnemy(false);
			insertBezierControlPointPlaceholder(nextBezierControlPointPlaceholderDesiredID, mouseWorldPos.x, mouseWorldPos.y);
		} else {
			setPlacingNewEnemy(false);
			std::shared_ptr<EnemyEntityPlaceholder> enemy = std::make_shared<EnemyEntityPlaceholder>(nextPlaceholderID, registry, *spriteLoader, *levelPack, *queue);
			nextPlaceholderID++;
			undoStack.execute(UndoableCommand(
				[this, mouseWorldPos, enemy]() {
				mostRecentNewEnemyPlaceholderID = enemy->getID();
				enemy->moveTo(mouseWorldPos.x, mouseWorldPos.y);
				enemy->spawnVisualEntity();
				nonplayerPlaceholders[enemy->getID()] = enemy;
				selectPlaceholder(enemy);
				updateEntityPlaceholdersList();
			},
				[this, enemy]() {
				deletePlaceholder(enemy->getID());
			}));
		}
	} else if (manuallySettingPlaceholderPosition) {
		setManuallySettingPlaceholderPosition(selectedPlaceholder, false);
		float oldX = selectedPlaceholder->getX();
		float oldY = selectedPlaceholder->getY();
		undoStack.execute(UndoableCommand(
			[this, &selectedPlaceholder = this->selectedPlaceholder, mouseWorldPos]() {
			selectedPlaceholder->moveTo(mouseWorldPos.x, mouseWorldPos.y);
			if (selectedPlaceholder == this->selectedPlaceholder) {
				setEntityPlaceholderXWidgetValue(selectedPlaceholder->getX());
				setEntityPlaceholderYWidgetValue(selectedPlaceholder->getY());
			}
			if (editingBezierControlPoints) {
				// Update entity placeholder list because control point entries have their coordinates
				updateEntityPlaceholdersList();
			}
		},
			[this, &selectedPlaceholder = this->selectedPlaceholder, oldX, oldY]() {
			selectedPlaceholder->moveTo(oldX, oldY);
			if (selectedPlaceholder == this->selectedPlaceholder) {
				setEntityPlaceholderXWidgetValue(selectedPlaceholder->getX());
				setEntityPlaceholderYWidgetValue(selectedPlaceholder->getY());
			}
			if (editingBezierControlPoints) {
				// Update entity placeholder list because control point entries have their coordinates
				updateEntityPlaceholdersList();
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
		for (auto p : nonplayerPlaceholders) {
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
	preTestCameraCenter = window->getView().getCenter();
	preTestCameraZoom = cameraZoom;

	std::string message;
	bool good = true;
	for (auto p : nonplayerPlaceholders) {
		EnemyEntityPlaceholder* enemyPlaceholder = dynamic_cast<EnemyEntityPlaceholder*>(p.second.get());
		if (enemyPlaceholder) {
			good = good && enemyPlaceholder->legalCheck(message, *levelPack, *spriteLoader);
		} else if (dynamic_cast<EMPTestEntityPlaceholder*>(p.second.get())) {
			EMPTestEntityPlaceholder* empTestPlaceholder = dynamic_cast<EMPTestEntityPlaceholder*>(p.second.get());
			good = good && empTestPlaceholder->legalCheck(message, *levelPack, *spriteLoader);
		}
	}
	clearLogs();
	if (message != "") {
		logMessage(message);
	}

	if (good) {
		testInProgress = true;
		startAndEndTest->setText("End test");

		if (!editingBezierControlPoints) {
			playerPlaceholder->runTest(registryMutex);
		}
		for (auto p : nonplayerPlaceholders) {
			p.second->runTest(registryMutex);
		}

		deselectPlaceholder();
		leftPanel->setVisible(false);
		externalEndTest->setVisible(true);

		lookAt(MAP_WIDTH / 2.0f, -MAP_HEIGHT / 2.0f);
		setCameraZoom(1);

        if (bottomPanel->isVisible()) {
            toggleLogsDisplay();
        }
    } else if (!bottomPanel->isVisible()) {
        toggleLogsDisplay();
    }
}

void GameplayTestWindow::endGameplayTest() {
	testInProgress = false;
	startAndEndTest->setText("Start test");
	leftPanel->setVisible(true);
	externalEndTest->setVisible(false);

	lookAt(preTestCameraCenter.x, preTestCameraCenter.y);
	setCameraZoom(preTestCameraZoom);

	playerPlaceholder->endTest(registryMutex);
	for (auto p : nonplayerPlaceholders) {
		p.second->endTest(registryMutex);
	}

	// Delete all entities except level manager
	std::lock_guard<std::mutex> lock(*registryMutex);
	uint32_t levelManager = registry.attachee<LevelManagerTag>();
	registry.each([&](uint32_t entity) {
		if (entity != levelManager) {
			registry.destroy(entity);
		}
	});
	// Respawn visual entities
	playerPlaceholder->spawnVisualEntity();
	for (auto p : nonplayerPlaceholders) {
		p.second->spawnVisualEntity();
	}

	levelPack->deleteTemporaryEditorObjecs();
}

void GameplayTestWindow::selectPlaceholder(std::shared_ptr<EntityPlaceholder> placeholder) {
    if (testInProgress) {
        return;
    }

	deselectPlaceholder();
	selectedPlaceholder = placeholder;
	selectedPlaceholder->onSelection();

	if (dynamic_cast<BezierControlPointPlaceholder*>(placeholder.get()) != nullptr) {
		ignoreSignal = true;
		rightPanel->setVisible(true);
		deleteEnemyPlaceholder->setEnabled(true);
		setEnemyPlaceholderTestMode->setVisible(false);
		testModeIDLabel->setVisible(false);
		testModeID->setVisible(false);
		showMovementPathLabel->setVisible(false);
		showMovementPath->setVisible(false);
		setEntityPlaceholderXWidgetValue(placeholder->getX());
		setEntityPlaceholderYWidgetValue(placeholder->getY());
		entityPlaceholdersList->getListBox()->setSelectedItemById(std::to_string(placeholder->getID()));
		ignoreSignal = false;
	} else if (dynamic_cast<EMPTestEntityPlaceholder*>(placeholder.get()) != nullptr) {
		ignoreSignal = true;
		rightPanel->setVisible(true);
		deleteEnemyPlaceholder->setEnabled(true);
		setEnemyPlaceholderTestMode->setVisible(false);
		testModeIDLabel->setVisible(false);
		testModeID->setVisible(false);
		showMovementPathLabel->setVisible(true);
		showMovementPath->setVisible(true);
		setEntityPlaceholderXWidgetValue(placeholder->getX());
		setEntityPlaceholderYWidgetValue(placeholder->getY());
		entityPlaceholdersList->getListBox()->setSelectedItemById(std::to_string(placeholder->getID()));
		ignoreSignal = false;
	} else {
		ignoreSignal = true;
		rightPanel->setVisible(true);
		bool isEnemyPlaceholder = (dynamic_cast<EnemyEntityPlaceholder*>(placeholder.get()) != nullptr);
		selectedPlaceholderIsPlayer = !isEnemyPlaceholder;
		deleteEnemyPlaceholder->setEnabled(isEnemyPlaceholder);
		setEnemyPlaceholderTestMode->setVisible(isEnemyPlaceholder);
		testModeIDLabel->setVisible(isEnemyPlaceholder);
		testModeID->setVisible(isEnemyPlaceholder);
		setEntityPlaceholderXWidgetValue(placeholder->getX());
		setEntityPlaceholderYWidgetValue(placeholder->getY());
		if (isEnemyPlaceholder) {
			testModeID->getListBox()->removeAllItems();

			auto enemyPlaceholder = dynamic_cast<EnemyEntityPlaceholder*>(placeholder.get());
			if (enemyPlaceholder->getTestMode() == EnemyEntityPlaceholder::ENEMY) {
				testModeIDLabel->setText("Enemy ID");
				for (auto it = levelPack->getEnemyIteratorBegin(); it != levelPack->getEnemyIteratorEnd(); it++) {
					std::string ignoreThis; // useless var
					if (it->second->legal(ignoreThis)) {
						testModeID->getListBox()->addItem(it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
					}
				}
				showMovementPathLabel->setVisible(false);
				showMovementPath->setVisible(false);
			} else if (enemyPlaceholder->getTestMode() == EnemyEntityPlaceholder::PHASE) {
				testModeIDLabel->setText("Phase ID");
				for (auto it = levelPack->getEnemyPhaseIteratorBegin(); it != levelPack->getEnemyPhaseIteratorEnd(); it++) {
					std::string ignoreThis; // useless var
					if (it->second->legal(ignoreThis)) {
						testModeID->getListBox()->addItem(it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
					}
				}
				showMovementPathLabel->setVisible(false);
				showMovementPath->setVisible(false);
			} else if (enemyPlaceholder->getTestMode() == EnemyEntityPlaceholder::ATTACK_PATTERN) {
				testModeIDLabel->setText("Attack pattern ID");
				for (auto it = levelPack->getAttackPatternIteratorBegin(); it != levelPack->getAttackPatternIteratorEnd(); it++) {
					std::string ignoreThis; // useless var
					if (it->second->legal(ignoreThis)) {
						testModeID->getListBox()->addItem(it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
					}
				}
				showMovementPathLabel->setVisible(true);
				showMovementPath->setVisible(true);
			} else if (enemyPlaceholder->getTestMode() == EnemyEntityPlaceholder::ATTACK) {
				testModeIDLabel->setText("Attack ID");
				for (auto it = levelPack->getAttackIteratorBegin(); it != levelPack->getAttackIteratorEnd(); it++) {
					std::string ignoreThis; // useless var
					if (it->second->legal(*levelPack, *spriteLoader, ignoreThis)) {
						testModeID->getListBox()->addItem(it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
					}
				}
				showMovementPathLabel->setVisible(false);
				showMovementPath->setVisible(false);
			}

			testModeID->getListBox()->setSelectedItemById(std::to_string(static_cast<int>(enemyPlaceholder->getTestModeID())));
		} else {
			showMovementPathLabel->setVisible(false);
			showMovementPath->setVisible(false);
		}
		entityPlaceholdersList->getListBox()->setSelectedItemById(std::to_string(placeholder->getID()));
		ignoreSignal = false;
	}

	ignoreSignal = true;
	if (editingBezierControlPoints) {
		addControlPointPlaceholderAbove->setText("Add above");
		addControlPointPlaceholderBelow->setText("Add below");
	}
	if (showMovementPath->isVisible()) {
		showMovementPath->setChecked(placeholderMovementPaths.count(placeholder->getID()) > 0);
	} else if (placeholderMovementPaths.count(placeholder->getID()) > 0) {
		placeholderMovementPaths.erase(placeholder->getID());
	}
	ignoreSignal = false;
}

void GameplayTestWindow::deselectPlaceholder() {
	if (selectedPlaceholder) {
		selectedPlaceholder->onDeselection();
	}

	rightPanel->setVisible(false);
	currentCursor = nullptr;
	selectedPlaceholder = nullptr;
	entityPlaceholdersList->getListBox()->deselectItem();
	deleteEnemyPlaceholder->setEnabled(false);

	if (editingBezierControlPoints) {
		addControlPointPlaceholderAbove->setText("Add as first");
		addControlPointPlaceholderBelow->setText("Add as last");
	}
}

void GameplayTestWindow::deletePlaceholder(int placeholderID) {
	assert(placeholderID != playerPlaceholder->getID()); // Player placeholder can't be deleted

	if (selectedPlaceholder && placeholderID == selectedPlaceholder->getID()) {
		deselectPlaceholder();

		// If the selected placeholder was the only one in the list, ...
		if (nonplayerPlaceholders.size() == 1) {
			// If not editing bezier control points, select the player placeholder
			if (!editingBezierControlPoints) {
				selectPlaceholder(playerPlaceholder);
			}
		} else if (std::prev(nonplayerPlaceholders.end())->first == placeholderID) {
			// If the selected placeholder was the last one in the list, try to select the 2nd to last

			selectPlaceholder(std::prev(std::prev(nonplayerPlaceholders.end()))->second);
		} else {
			// Try to select the next placeholder in the list
			for (auto it = nonplayerPlaceholders.begin(); it != nonplayerPlaceholders.end(); it++) {
				if (it->first == placeholderID) {
					if (std::next(it) != nonplayerPlaceholders.end()) {
						selectPlaceholder(std::next(it)->second);
					}
					break;
				}
			}
		}
	}

	if (nonplayerPlaceholders.count(placeholderID) > 0) {
		nonplayerPlaceholders[placeholderID]->removePlaceholder(registryMutex);
		nonplayerPlaceholders.erase(placeholderID);
		entityPlaceholdersList->getListBox()->removeItemById(std::to_string(placeholderID));
	}

	updateEntityPlaceholdersList();
}

void GameplayTestWindow::updateEntityPlaceholdersList() {
	ignoreSignal = true;
	entityPlaceholdersList->getListBox()->removeAllItems();
	if (!editingBezierControlPoints) {
		entityPlaceholdersList->getListBox()->addItem("[id=" + std::to_string(playerPlaceholder->getID()) + "] Player", std::to_string(playerPlaceholder->getID()));
	}
	int i = 0;
	for (auto p : nonplayerPlaceholders) {
		EnemyEntityPlaceholder* enemyPlaceholder = dynamic_cast<EnemyEntityPlaceholder*>(p.second.get());
		if (enemyPlaceholder) {
			std::string text = "";
			if (!enemyPlaceholder->testModeIDSet()) {
				text = "Unset";
			} else {
				if (enemyPlaceholder->getTestMode() == EnemyEntityPlaceholder::ENEMY) {
					text = "Enemy";
				} else if (enemyPlaceholder->getTestMode() == EnemyEntityPlaceholder::PHASE) {
					text = "Phase";
				} else if (enemyPlaceholder->getTestMode() == EnemyEntityPlaceholder::ATTACK_PATTERN) {
					text = "A.Pattern";
				} else {
					text = "Attack";
				}
				text += " " + std::to_string(enemyPlaceholder->getTestModeID());
			}
			entityPlaceholdersList->getListBox()->addItem("[id=" + std::to_string(p.first) + "] " + text, std::to_string(p.first));
		} else if (dynamic_cast<EMPTestEntityPlaceholder*>(p.second.get())) {
			EMPTestEntityPlaceholder* empTestPlaceholder = dynamic_cast<EMPTestEntityPlaceholder*>(p.second.get());
			if (empTestPlaceholder->empIsFromAttack()) {
				entityPlaceholdersList->getListBox()->addItem("[id=" + std::to_string(p.first) + "] EMP " + std::to_string(empTestPlaceholder->getEMPID()) + " Attack " + std::to_string(empTestPlaceholder->getSourceID()), std::to_string(p.first));
			} else {
				entityPlaceholdersList->getListBox()->addItem("[id=" + std::to_string(p.first) + "] EMP " + std::to_string(empTestPlaceholder->getEMPID()) + " A.Pattern " + std::to_string(empTestPlaceholder->getSourceID()), std::to_string(p.first));
			}
		} else if (dynamic_cast<BezierControlPointPlaceholder*>(p.second.get())) {
			BezierControlPointPlaceholder* cpPlaceholder = dynamic_cast<BezierControlPointPlaceholder*>(p.second.get());
			// Display the x/y relative to the first control point
			entityPlaceholdersList->getListBox()->addItem("[" + std::to_string(i) + "] x=" + std::to_string(cpPlaceholder->getX() - nonplayerPlaceholders[0]->getX()) + " y=" + std::to_string(cpPlaceholder->getY() - nonplayerPlaceholders[0]->getY()), std::to_string(p.first));
		}
		i++;
	}
	// Reslect the selectPlaceholder, if any
	if (selectedPlaceholder) {
		entityPlaceholdersList->getListBox()->setSelectedItemById(std::to_string(selectedPlaceholder->getID()));
	}
	ignoreSignal = false;

	if (editingBezierControlPoints) {
		showBezierMovementPath();
	} else {
		// Recalculate movement paths
		for (auto it = placeholderMovementPaths.begin(); it != placeholderMovementPaths.end(); it++) {
			showPlaceholderMovementPath(nonplayerPlaceholders[it->first]);
		}
	}
}

void GameplayTestWindow::setPlacingNewEnemy(bool placingNewEnemy) {
	this->placingNewEnemy = placingNewEnemy;
	if (placingNewEnemy) {
		currentCursor = movingEnemyPlaceholderCursor;
	} else if (!manuallySettingPlaceholderPosition) {
		currentCursor = nullptr;
	}
}

void GameplayTestWindow::setManuallySettingPlaceholderPosition(std::shared_ptr<EntityPlaceholder> placeholder,  bool manuallySettingPlaceholderPosition) {
	this->manuallySettingPlaceholderPosition = manuallySettingPlaceholderPosition;
	if (manuallySettingPlaceholderPosition) {
		bool isPlayerPlaceholder = (dynamic_cast<PlayerEntityPlaceholder*>(placeholder.get()) != nullptr);
		if (isPlayerPlaceholder) {
			currentCursor = movingPlayerPlaceholderCursor;
		} else {
			currentCursor = movingEnemyPlaceholderCursor;
		}
	} else if (!placingNewEnemy) {
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
	boost::posix_time::ptime timeLocal = boost::posix_time::second_clock::local_time();
	std::stringstream stream;
	stream << timeLocal.time_of_day();
	logs->setText(logs->getText() + "\n[" + stream.str() + "] " + message);
}

void GameplayTestWindow::showPlaceholderMovementPath(std::shared_ptr<EntityPlaceholder> placeholder) {
	placeholderMovementPaths[placeholder->getID()] = placeholder->getMovementPath(MOVEMENT_PATH_TIME_RESOLUTION, playerPlaceholder->getX(), playerPlaceholder->getY());
}

void GameplayTestWindow::showBezierMovementPath() {
	//TODO: different colors
	std::vector<sf::Vector2f> points;
	for (auto it = nonplayerPlaceholders.begin(); it != nonplayerPlaceholders.end(); it++) {
		points.push_back(sf::Vector2f(it->second->getX(), it->second->getY()));
	}
	std::shared_ptr<EMPAction> empa = std::make_shared<MoveCustomBezierEMPA>(points, editingBezierEMPALifespan);
	bezierMovementPath = generateVertexArray(empa, MOVEMENT_PATH_TIME_RESOLUTION, 0, 0, 0, 0);
	bezierMovementPath.setPrimitiveType(sf::PrimitiveType::Lines);
	// Negate y value of each vertex since our render system uses negative y's and then map to screen coordinates
	for (int i = 0; i < bezierMovementPath.getVertexCount(); i++) {
		bezierMovementPath[i].position = sf::Vector2f(bezierMovementPath[i].position.x, -bezierMovementPath[i].position.y);
	}
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

void GameplayTestWindow::EntityPlaceholder::endTest(std::shared_ptr<std::mutex> registryMutex) {
	std::lock_guard<std::mutex> lock(*registryMutex);
	if (registry.valid(testEntity)) {
		registry.destroy(testEntity);
	}
}

bool GameplayTestWindow::EntityPlaceholder::wasClicked(int worldX, int worldY) {
	return std::sqrt((worldX - x)*(worldX - x) + (worldY - y)*(worldY - y)) <= CLICK_HITBOX_SIZE;
}

void GameplayTestWindow::EntityPlaceholder::removePlaceholder(std::shared_ptr<std::mutex> registryMutex) {
	std::lock_guard<std::mutex> lock(*registryMutex);
	if (registry.valid(visualEntity)) {
		registry.destroy(visualEntity);
	}
	if (registry.valid(testEntity)) {
		registry.destroy(testEntity);
	}
}

void GameplayTestWindow::EntityPlaceholder::onSelection() {
	if (registry.valid(visualEntity)) {
		registry.get<SpriteComponent>(visualEntity).setAnimatable(spriteLoader, Animatable("Selected Placeholder", "Default", true, LOCK_ROTATION), false);
	}
}

void GameplayTestWindow::EntityPlaceholder::onDeselection() {
	if (registry.valid(visualEntity)) {
		registry.get<SpriteComponent>(visualEntity).setAnimatable(spriteLoader, getVisualEntityAnimatable(), false);
	}
}

void GameplayTestWindow::PlayerEntityPlaceholder::spawnVisualEntity() {
	assert(!registry.valid(visualEntity));
	visualEntity = registry.create();
	registry.assign<PositionComponent>(visualEntity, x, y);
	registry.assign<SpriteComponent>(visualEntity, spriteLoader, getVisualEntityAnimatable(), true, PLAYER_LAYER, 0);
}

Animatable GameplayTestWindow::PlayerEntityPlaceholder::getVisualEntityAnimatable() {
	return Animatable("Player Placeholder", "Default", true, LOCK_ROTATION);
}

sf::VertexArray GameplayTestWindow::PlayerEntityPlaceholder::getMovementPath(float timeResolution, float playerX, float playerY) {
	// No path for this
	return sf::VertexArray();
}

void GameplayTestWindow::PlayerEntityPlaceholder::runTest(std::shared_ptr<std::mutex> registryMutex) {
	std::lock_guard<std::mutex> lock(*registryMutex);
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
		pemp0->setHitboxRadius(20);
		pemp0->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>(1, 0, 0));
		pemp0->insertAction(0, std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, 700, 2), std::make_shared<ConstantTFV>(PI / 2.0f), 2.0f));
		pemp0->setOnCollisionAction(PIERCE_ENTITY);
		playerAP->addAttackID(0.1f, playerAttack1->getID());

		auto bombAP = levelPack.createTempAttackPattern();
		for (int i = 0; i < 10; i++) {
			auto bombAttack1 = levelPack.createTempAttack();
			auto b1emp0 = bombAttack1->searchEMP(0);
			b1emp0->setAnimatable(Animatable("Player Bullet", "Default", true, LOCK_ROTATION));
			b1emp0->setHitboxRadius(7.5f);
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
	registry.assign<PositionComponent>(testEntity, x - params.getHitboxPosX(), y - params.getHitboxPosY());
	registry.assign<SpriteComponent>(testEntity, PLAYER_LAYER, 0);
}

void GameplayTestWindow::EnemyEntityPlaceholder::runTest(std::shared_ptr<std::mutex> registryMutex) {
	std::lock_guard<std::mutex> lock(*registryMutex);
	registry.destroy(visualEntity);

	EnemySpawnInfo info;
	if (testMode == ENEMY) {
		info = EnemySpawnInfo(testModeID, x, y, std::vector<std::pair<std::shared_ptr<Item>, int>>());
	} else if (testMode == PHASE) {
		std::shared_ptr<EditorEnemy> enemy = levelPack.createTempEnemy();
		EntityAnimatableSet enemyAnimatableSet(Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT), Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT), Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT));
		enemy->setHitboxRadius(20);
		enemy->setHealth(2000000000);
		enemy->addPhaseID(0, std::make_shared<TimeBasedEnemyPhaseStartCondition>(0), testModeID, enemyAnimatableSet);
		info = EnemySpawnInfo(enemy->getID(), x, y, std::vector<std::pair<std::shared_ptr<Item>, int>>());
	} else if (testMode == ATTACK_PATTERN) {
		std::shared_ptr<EditorEnemyPhase> phase = levelPack.createTempEnemyPhase();
		phase->addAttackPatternID(0, testModeID);
		// TODO: make this customizable; must be >= 0; this is the delay before restarting the attack pattern loop
		phase->setAttackPatternLoopDelay(3.0f);

		std::shared_ptr<EditorEnemy> enemy = levelPack.createTempEnemy();
		EntityAnimatableSet enemyAnimatableSet(Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT), Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT), Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT));
		enemy->setHitboxRadius(20);
		enemy->setHealth(2000000000);
		enemy->addPhaseID(0, std::make_shared<TimeBasedEnemyPhaseStartCondition>(0), phase->getID(), enemyAnimatableSet);
		info = EnemySpawnInfo(enemy->getID(), x, y, std::vector<std::pair<std::shared_ptr<Item>, int>>());
	} else {
		std::shared_ptr<EditorAttackPattern> attackPattern = levelPack.createTempAttackPattern();
		attackPattern->addAttackID(0, testModeID);

		std::shared_ptr<EditorEnemyPhase> phase = levelPack.createTempEnemyPhase();
		phase->addAttackPatternID(0, attackPattern->getID());
		// TODO: make this customizable; must be > 0; this is the time between each attack
		phase->setAttackPatternLoopDelay(1.0f);

		std::shared_ptr<EditorEnemy> enemy = levelPack.createTempEnemy();
		EntityAnimatableSet enemyAnimatableSet(Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT), Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT), Animatable("Enemy Placeholder", "Default", true, ROTATE_WITH_MOVEMENT));
		enemy->setHitboxRadius(20);
		enemy->setHealth(2000000000);
		enemy->addPhaseID(0, std::make_shared<TimeBasedEnemyPhaseStartCondition>(0), phase->getID(), enemyAnimatableSet);
		info = EnemySpawnInfo(enemy->getID(), x, y, std::vector<std::pair<std::shared_ptr<Item>, int>>());
	}
	info.spawnEnemy(spriteLoader, levelPack, registry, queue);
}

void GameplayTestWindow::EnemyEntityPlaceholder::spawnVisualEntity() {
	assert(!registry.valid(visualEntity));
	visualEntity = registry.create();
	registry.assign<PositionComponent>(visualEntity, x, y);
	registry.assign<SpriteComponent>(visualEntity, spriteLoader, getVisualEntityAnimatable(), true, PLAYER_LAYER, 0);
}

Animatable GameplayTestWindow::EnemyEntityPlaceholder::getVisualEntityAnimatable() {
	return Animatable("Enemy Placeholder", "Default", true, LOCK_ROTATION);
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
		} else if (testMode == ATTACK) {
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

sf::VertexArray GameplayTestWindow::EnemyEntityPlaceholder::getMovementPath(float timeResolution, float playerX, float playerY) {
	if (testMode == ATTACK_PATTERN && levelPack.hasAttackPattern(testModeID)) {
		auto attackPattern = levelPack.getAttackPattern(testModeID);
		//TODO: different colors
		sf::VertexArray va = generateVertexArray(attackPattern->getActions(), timeResolution, x, y, playerX, playerY);
		va.setPrimitiveType(sf::PrimitiveType::Lines);
		// Negate y value of each vertex since our render system uses negative y's and then map to screen coordinates
		for (int i = 0; i < va.getVertexCount(); i++) {
			va[i].position = sf::Vector2f(va[i].position.x, -va[i].position.y);
		}
		return va;
	} else {
		return sf::VertexArray();
	}
}

void GameplayTestWindow::EMPTestEntityPlaceholder::runTest(std::shared_ptr<std::mutex> registryMutex) {
	std::lock_guard<std::mutex> lock(*registryMutex);
	registry.destroy(visualEntity);

	queue.pushBack(std::make_unique<EMPSpawnFromNothingCommand>(registry, spriteLoader, emp, MPSpawnInformation{ false, NULL, sf::Vector2f(x, y) }, true, 0, -1, -1));
}

void GameplayTestWindow::EMPTestEntityPlaceholder::spawnVisualEntity() {
	assert(!registry.valid(visualEntity));
	visualEntity = registry.create();
	registry.assign<PositionComponent>(visualEntity, x, y);
	registry.assign<SpriteComponent>(visualEntity, spriteLoader, getVisualEntityAnimatable(), true, PLAYER_LAYER, 0);
}

Animatable GameplayTestWindow::EMPTestEntityPlaceholder::getVisualEntityAnimatable() {
	return Animatable("Enemy Placeholder", "Default", true, LOCK_ROTATION);
}

bool GameplayTestWindow::EMPTestEntityPlaceholder::legalCheck(std::string & message, LevelPack & levelPack, SpriteLoader & spriteLoader) {
	return emp->legal(levelPack, spriteLoader, message);
}

sf::VertexArray GameplayTestWindow::EMPTestEntityPlaceholder::getMovementPath(float timeResolution, float playerX, float playerY) {
	//TODO: different colors
	return generateVertexArray(emp->getActions(), timeResolution, x, y, playerX, playerY);
}

void GameplayTestWindow::BezierControlPointPlaceholder::runTest(std::shared_ptr<std::mutex> registryMutex) {
	//TODO
}

void GameplayTestWindow::BezierControlPointPlaceholder::spawnVisualEntity() {
	assert(!registry.valid(visualEntity));
	visualEntity = registry.create();
	registry.assign<PositionComponent>(visualEntity, x, y);
	registry.assign<SpriteComponent>(visualEntity, spriteLoader, getVisualEntityAnimatable(), true, PLAYER_LAYER, 0);
}

Animatable GameplayTestWindow::BezierControlPointPlaceholder::getVisualEntityAnimatable() {
	return Animatable("Enemy Placeholder", "Default", true, LOCK_ROTATION);
}

sf::VertexArray GameplayTestWindow::BezierControlPointPlaceholder::getMovementPath(float timeResolution, float playerX, float playerY) {
	// It's all wack because BezierControlPointPlaceholders don't have info on the entire bezier movement path so it has to be implemented as a
	// special case from GameplayTestWindow::showPlaceholderMovementPath(placeholder)
	return sf::VertexArray();
}
