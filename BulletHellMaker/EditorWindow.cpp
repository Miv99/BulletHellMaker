#include "EditorWindow.h"
#include "Components.h"
#include "Enemy.h"
#include "EnemyPhase.h"
#include "Level.h"
#include "AttackEditorPanel.h"
#include "AttackPatternEditorPanel.h"
#include "LevelPackObjectsListPanel.h"
#include <algorithm>
#include <boost/date_time.hpp>
#include <sstream>
#include <iterator>
#include <set>

EditorWindow::EditorWindow(std::shared_ptr<std::recursive_mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize, bool letterboxingEnabled, float renderInterval) :
	tguiMutex(tguiMutex), windowTitle(windowTitle), windowWidth(width), windowHeight(height), scaleWidgetsOnResize(scaleWidgetsOnResize), letterboxingEnabled(letterboxingEnabled), renderInterval(renderInterval) {
	std::lock_guard<std::recursive_mutex> lock(*tguiMutex);
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

	confirmationPanel->add(confirmationText);
	confirmationPanel->add(confirmationYes);
	confirmationPanel->add(confirmationNo);
}

void EditorWindow::start() {
	if (!window || !window->isOpen()) {
		// SFML requires the RenderWindow to be created in the thread

		std::lock_guard<std::recursive_mutex> lock(*tguiMutex);
		window = std::make_shared<sf::RenderWindow>(sf::VideoMode(windowWidth, windowHeight), windowTitle, sf::Style::Default);
		window->setKeyRepeatEnabled(false);
		window->setActive(true);
		gui->setTarget(*window);

		updateWindowView(window->getSize().x, window->getSize().y);
		onRenderWindowInitialization();
	}

	sf::Clock deltaClock;
	sf::Clock renderClock;

	// Main loop
	while (window->isOpen()) {
		// While behind in render updates, do physics updates
		float timeSinceLastRender = 0;
		while (timeSinceLastRender < RENDER_INTERVAL) {
			sf::Event event;
			while (!windowCloseQueued && window->pollEvent(event)) {
				if (windowCloseQueued || event.type == sf::Event::Closed) {
					closeSignal->publish();
					window->close();
					return;
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

				float dt = std::min(RENDER_INTERVAL, renderClock.restart().asSeconds());
				window->clear();
				render(dt);
				if (renderSignal) {
					renderSignal->publish(dt);
				}
				window->display();
			}

			if (windowCloseQueued) {
				return;
			}

			float dt = std::min(MAX_PHYSICS_DELTA_TIME, deltaClock.restart().asSeconds());
			timeSinceLastRender += dt;
			physicsUpdate(dt);
		}

		float dt = std::min(RENDER_INTERVAL, renderClock.restart().asSeconds());
		window->clear();
		render(dt);
		if (renderSignal) {
			renderSignal->publish(dt);
		}
		window->display();
	}
}

void EditorWindow::startAndHide() {
	if (!window || !window->isOpen()) {
		// SFML requires the RenderWindow to be created in the thread

		std::lock_guard<std::recursive_mutex> lock(*tguiMutex);
		window = std::make_shared<sf::RenderWindow>(sf::VideoMode(windowWidth, windowHeight), windowTitle, sf::Style::Default);
		window->setKeyRepeatEnabled(false);
		window->setActive(true);
		gui->setTarget(*window);

		updateWindowView(window->getSize().x, window->getSize().y);
		onRenderWindowInitialization();
	}
	hide();

	sf::Clock deltaClock;
	sf::Clock renderClock;

	// Main loop
	while (window->isOpen()) {
		// While behind in render updates, do physics updates
		float timeSinceLastRender = 0;
		while (timeSinceLastRender < RENDER_INTERVAL) {
			sf::Event event;
			while (!windowCloseQueued && window->pollEvent(event)) {
				if (windowCloseQueued || event.type == sf::Event::Closed) {
					closeSignal->publish();
					window->close();
					return;
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

				float dt = std::min(RENDER_INTERVAL, renderClock.restart().asSeconds());
				window->clear();
				render(dt);
				if (renderSignal) {
					renderSignal->publish(dt);
				}
				window->display();
			}

			float dt = std::min(MAX_PHYSICS_DELTA_TIME, deltaClock.restart().asSeconds());
			timeSinceLastRender += dt;
			physicsUpdate(dt);
		}

		float dt = std::min(RENDER_INTERVAL, renderClock.restart().asSeconds());
		window->clear();
		render(dt);
		if (renderSignal) {
			renderSignal->publish(dt);
		}
		window->display();
	}
}

void EditorWindow::close() {
	if (window && window->isOpen()) {
		windowCloseQueued = true;
	}
}

void EditorWindow::hide() {
	if (window) {
		window->setVisible(false);
	}
}

void EditorWindow::show() {
	if (window) {
		window->setVisible(true);
	}
}

std::shared_ptr<entt::SigH<void(bool)>> EditorWindow::promptConfirmation(std::string message) {
	// Don't allow 2 confirmation prompts at the same time
	if (confirmationPanelOpen) {
		return nullptr;
	}

	std::shared_ptr<entt::SigH<void(bool)>> confirmationSignal = std::make_shared<entt::SigH<void(bool)>>();

	// Disable all widgets
	widgetsToBeEnabledAfterConfirmationPrompt.clear();
	for (std::shared_ptr<tgui::Widget> ptr : gui->getWidgets()) {
		if (ptr->isEnabled()) {
			widgetsToBeEnabledAfterConfirmationPrompt.push_back(ptr);
		}
		ptr->setEnabled(false);
	}

	confirmationYes->connect("Pressed", [this, confirmationSignal]() {
		confirmationSignal->publish(true);
		confirmationSignal->sink().disconnect();
		closeConfirmationPanel();
	});
	confirmationNo->connect("Pressed", [this, confirmationSignal]() {
		confirmationSignal->publish(false);
		confirmationSignal->sink().disconnect();
		closeConfirmationPanel();
	});

	confirmationText->setText(message);
	gui->add(confirmationPanel);
	confirmationPanelOpen = true;

	return confirmationSignal;
}

void EditorWindow::addPopupWidget(std::shared_ptr<tgui::Widget> popup, float preferredX, float preferredY, float preferredWidth, float preferredHeight, float maxWidthFraction, float maxHeightFraction) {
	if (this->popup) {
		closePopupWidget();
	}

	// Set popup position
	popup->setPosition(preferredX, preferredY);
	// Set popup size
	float popupWidth = std::min(preferredWidth, windowWidth * maxWidthFraction);
	float popupHeight = std::min(preferredHeight, windowHeight * maxHeightFraction);
	// Popup width/height can't be such that part of the popup goes past the window width/height
	float oldWidth = popupWidth;
	popupWidth = std::min(popupWidth, windowWidth - popup->getAbsolutePosition().x);
	float oldHeight = popupHeight;
	popupHeight = std::min(popupHeight, windowHeight - popup->getAbsolutePosition().y);
	// If it was going to go past the window width/height, move the popup back the amount it was going to go past the window width/height
	popup->setPosition(std::max(0.0f, popup->getPosition().x - (oldWidth - popupWidth)), std::max(0.0f, popup->getPosition().y - (oldHeight - popupHeight)));
	popup->setSize(oldWidth, oldHeight);

	this->popup = popup;
	popupContainer = nullptr;
	gui->add(popup);
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
	if (popupContainer) {
		popupContainer->remove(popup);
	} else {
		// If not in the popupContainer, it must be in the Gui
		gui->remove(popup);
	}
	popup = nullptr;
	popupContainer = nullptr;
}

int EditorWindow::addVertexArray(sf::VertexArray vertexArray) {
	int id = nextVertexArrayID;
	vertexArrays[id] = vertexArray;
	nextVertexArrayID++;
	return id;
}

void EditorWindow::removeVertexArray(int id) {
	if (vertexArrays.count(id) > 0) {
		vertexArrays.erase(id);
	}
}

void EditorWindow::modifyVertexArray(int id, sf::VertexArray newVertexArray) {
	if (vertexArrays.count(id) > 0) {
		vertexArrays[id] = newVertexArray;
	}
}

void EditorWindow::removeAllVertexArrays() {
	vertexArrays.clear();
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
	std::lock_guard<std::recursive_mutex> lock(*tguiMutex);
	gui->draw();

	for (auto it = vertexArrays.begin(); it != vertexArrays.end(); it++) {
		window->draw(it->second);
	}
}

bool EditorWindow::handleEvent(sf::Event event) {
	std::lock_guard<std::recursive_mutex> lock(*tguiMutex);

	if (event.type == sf::Event::MouseMoved) {
		mousePos.x = event.mouseMove.x;
		mousePos.y = event.mouseMove.y;
	} else if (event.type == sf::Event::MouseButtonPressed) {
		lastMousePressPos = mousePos;
	}

	// Disable keyboard events when confirmation panel is open
	if (!(confirmationPanelOpen && (event.type == sf::Event::TextEntered || event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased))) {
		gui->handleEvent(event);
		return false;
	} else {
		return true;
	}
}

void EditorWindow::onRenderWindowInitialization() {
}

void EditorWindow::closeConfirmationPanel() {
	gui->remove(confirmationPanel);
	confirmationPanelOpen = false;

	for (auto ptr : widgetsToBeEnabledAfterConfirmationPrompt) {
		ptr->setEnabled(true);
	}
	widgetsToBeEnabledAfterConfirmationPrompt.clear();
}

MainEditorWindow::MainEditorWindow(std::shared_ptr<std::recursive_mutex> tguiMutex, std::string windowTitle, int width, int height, bool scaleWidgetsOnResize, bool letterboxingEnabled, float renderInterval)
	: EditorWindow(tguiMutex, windowTitle, width, height, scaleWidgetsOnResize, letterboxingEnabled, renderInterval) {
	std::lock_guard<std::recursive_mutex> lock(*tguiMutex);

	leftPanel = TabsWithPanel::create(*this);
	leftPanel->setPosition(0, 0);
	leftPanel->setSize("20%", "100%");
	leftPanel->setVisible(true);
	leftPanel->setMoreTabsListAlignment(TabsWithPanel::MoreTabsListAlignment::Right);
	gui->add(leftPanel);

	{
		// Attacks tab in left panel
		attacksListView = AttacksListView::create(*this, clipboard);
		attacksListPanel = LevelPackObjectsListPanel::create(*this, clipboard);
		populateLeftPanelLevelPackObjectListPanel(attacksListView, attacksListPanel,
			[this]() { 
				this->createAttack();
			}, 
			[this](int id) { 
				this->openLeftPanelAttack(id); 
			}
		);
		leftPanel->addTab(LEFT_PANEL_ATTACK_LIST_TAB_NAME, attacksListPanel, true);
	}
	{
		// Attack patterns tab in left panel
		attackPatternsListView = AttackPatternsListView::create(*this, clipboard);
		attackPatternsListPanel = LevelPackObjectsListPanel::create(*this, clipboard);
		populateLeftPanelLevelPackObjectListPanel(attackPatternsListView, attackPatternsListPanel,
			[this]() {
			this->createAttackPattern();
			},
			[this](int id) {
				this->openLeftPanelAttackPattern(id);
			}
		);
		leftPanel->addTab(LEFT_PANEL_ATTACK_PATTERN_LIST_TAB_NAME, attackPatternsListPanel, true);
	}

	mainPanel = TabsWithPanel::create(*this);
	mainPanel->setPosition(tgui::bindRight(leftPanel), tgui::bindTop(leftPanel));
	mainPanel->setSize("80%", "100%");
	mainPanel->setVisible(true);
	mainPanel->setMoreTabsListAlignment(TabsWithPanel::MoreTabsListAlignment::Left);
	gui->add(mainPanel);

	clipboardNotification = TextNotification::create();
	clipboardNotification->setPosition(0, tgui::bindBottom(leftPanel) - tgui::bindHeight(clipboardNotification));
	gui->add(clipboardNotification);

	clipboard.getOnCopy()->sink().connect<MainEditorWindow, &MainEditorWindow::showClipboardResult>(this);
	clipboard.getOnPaste()->sink().connect<MainEditorWindow, &MainEditorWindow::showClipboardResult>(this);
}

MainEditorWindow::~MainEditorWindow() {
	clipboard.getOnCopy()->sink().disconnect(this);
	clipboard.getOnPaste()->sink().disconnect(this);
	previewWindow->close();
}

void MainEditorWindow::loadLevelPack(std::string levelPackName) {
	audioPlayer = std::make_shared<AudioPlayer>();
	levelPack = std::make_shared<LevelPack>(*audioPlayer, levelPackName);
	spriteLoader = levelPack->createSpriteLoader();
	spriteLoader->preloadTextures();

	attacksListPanel->setLevelPack(levelPack.get());
	attacksListView->setLevelPack(levelPack.get());
	attacksListView->reload();

	attackPatternsListPanel->setLevelPack(levelPack.get());
	attackPatternsListView->setLevelPack(levelPack.get());
	attackPatternsListView->reload();

	// TODO: add more left panel panels to here

	// TODO: window size from settings
	previewWindow = std::make_shared<LevelPackObjectPreviewWindow>(tguiMutex, "Preview", 1024, 768, levelPackName);
	boost::thread previewWindowThread = boost::thread(&LevelPackObjectPreviewWindow::start, &(*previewWindow));
	previewWindowThread.detach();
}

void MainEditorWindow::overwriteAttacks(std::vector<std::shared_ptr<EditorAttack>> attacks, UndoStack* undoStack) {
	std::vector<std::shared_ptr<EditorAttack>> oldAttacks;
	for (std::shared_ptr<EditorAttack> attack : attacks) {
		int id = attack->getID();
		if (unsavedAttacks.count(id) > 0) {
			oldAttacks.push_back(std::dynamic_pointer_cast<EditorAttack>(unsavedAttacks[id]->clone()));
		} else {
			oldAttacks.push_back(std::make_shared<EditorAttack>(levelPack->getAttack(id)));
		}
	}

	if (undoStack) {
		undoStack->execute(UndoableCommand([this, attacks]() {
			for (std::shared_ptr<EditorAttack> attack : attacks) {
				int id = attack->getID();

				// Overwrite existing EditorAttack
				if (unsavedAttacks.count(id) > 0) {
					std::dynamic_pointer_cast<EditorAttack>(unsavedAttacks[id])->load(attack->format());
				} else {
					unsavedAttacks[id] = std::dynamic_pointer_cast<LevelPackObject>(std::make_shared<EditorAttack>(attack));
				}

				reloadAttackTab(id);
				attacksListView->reload();
			}
		}, [this, oldAttacks]() {
			for (std::shared_ptr<EditorAttack> attack : oldAttacks) {
				int id = attack->getID();

				// Overwrite existing EditorAttack
				if (unsavedAttacks.count(id) > 0) {
					std::dynamic_pointer_cast<EditorAttack>(unsavedAttacks[id])->load(attack->format());
				} else {
					unsavedAttacks[id] = std::dynamic_pointer_cast<LevelPackObject>(std::make_shared<EditorAttack>(attack));
				}

				reloadAttackTab(id);
				attacksListView->reload();
			}
		}));
	} else {
		// Overwrite without pushing to an undo stack

		for (std::shared_ptr<EditorAttack> attack : attacks) {
			int id = attack->getID();

			// Overwrite existing EditorAttack
			if (unsavedAttacks.count(id) > 0) {
				std::dynamic_pointer_cast<EditorAttack>(unsavedAttacks[id])->load(attack->format());
			} else {
				unsavedAttacks[id] = std::dynamic_pointer_cast<LevelPackObject>(std::make_shared<EditorAttack>(attack));
			}

			reloadAttackTab(id);
			attacksListView->reload();
		}
	}
}

void MainEditorWindow::openLeftPanelEnemyPhase(int enemyPhaseID) {
	// TODO
}

void MainEditorWindow::openLeftPanelPlayer() {
	// TODO
}

bool MainEditorWindow::handleEvent(sf::Event event) {
	if (EditorWindow::handleEvent(event)) {
		return true;
	}

	if (leftPanel->mouseOnWidget(lastMousePressPos)) {
		return leftPanel->handleEvent(event);
	} else if (mainPanel->mouseOnWidget(lastMousePressPos)) {
		return mainPanel->handleEvent(event);
	}
	return false;
}

void MainEditorWindow::showClipboardResult(std::string notification) {
	clipboardNotification->setText(notification);
}

void MainEditorWindow::populateLeftPanelLevelPackObjectListPanel(std::shared_ptr<LevelPackObjectsListView> listView, 
		std::shared_ptr<LevelPackObjectsListPanel> listPanel, std::function<void()> createLevelPackObject, std::function<void(int)> openLevelPackObjectTab) {
	// Add button
	auto addButton = tgui::Button::create();
	addButton->setText("+");
	addButton->setPosition(0, 0);
	addButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	addButton->connect("Pressed", [this, createLevelPackObject]() {
		createLevelPackObject();
	});
	listPanel->add(addButton);

	// Save all button
	auto saveAllButton = tgui::Button::create();
	saveAllButton->setText("S");
	saveAllButton->setPosition(tgui::bindRight(addButton), 0);
	saveAllButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	saveAllButton->connect("Pressed", [this, listView]() {
		listView->manualSaveAll();
	});
	listPanel->add(saveAllButton);

	// Sort button
	auto sortButton = tgui::Button::create();
	sortButton->setText("=");
	sortButton->setPosition(tgui::bindRight(saveAllButton), 0);
	sortButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	sortButton->connect("Pressed", [this, listView]() {
		listView->cycleSortOption();
	});
	listPanel->add(sortButton);

	// List view
	listView->setPosition(0, tgui::bindBottom(addButton));
	listView->setSize("100%", tgui::bindHeight(listPanel) - tgui::bindBottom(addButton));
	{
		// Right click menu
		// Menu for single selection
		auto rightClickMenuPopupSingleSelection = createMenuPopup({
			std::make_pair("Open", [this, listView, openLevelPackObjectTab]() {
				openLevelPackObjectTab(listView->getLevelPackObjectIDFromIndex(listView->getListView()->getSelectedItemIndex()));
			}),
			std::make_pair("Copy", [listView]() {
				listView->manualCopy();
			}),
			std::make_pair("Paste", [listView]() {
				listView->manualPaste();
			}),
			std::make_pair("Paste (override this)", [listView]() {
				listView->manualPaste2();
			}),
			std::make_pair("Save", [listView]() {
				listView->manualSave();
			}),
			std::make_pair("Delete", [listView]() {
				listView->manualDelete();
			})
			});
		// Menu for multiple selections
		auto rightClickMenuPopupMultiSelection = createMenuPopup({
			std::make_pair("Copy", [listView]() {
				listView->manualCopy();
			}),
			std::make_pair("Paste", [listView]() {
				listView->manualPaste();
			}),
			std::make_pair("Paste (override these)", [listView]() {
				listView->manualPaste2();
			}),
			std::make_pair("Save", [listView]() {
				listView->manualSave();
			}),
			std::make_pair("Delete", [listView]() {
				listView->manualDelete();
			})
			});
		listView->getListView()->connect("RightClicked", [this, listView, rightClickMenuPopupSingleSelection, rightClickMenuPopupMultiSelection](int index) {
			std::set<std::size_t> selectedItemIndices = listView->getListView()->getSelectedItemIndices();
			if (selectedItemIndices.find(index) != selectedItemIndices.end()) {
				// Right clicked a selected item

				// Open the corresponding menu
				if (selectedItemIndices.size() == 1) {
					addPopupWidget(rightClickMenuPopupSingleSelection, mousePos.x, mousePos.y, 150, rightClickMenuPopupSingleSelection->getSize().y);
				} else {
					addPopupWidget(rightClickMenuPopupMultiSelection, mousePos.x, mousePos.y, 150, rightClickMenuPopupMultiSelection->getSize().y);
				}
			} else {
				// Right clicked a nonselected item

				// Select the right clicked item
				listView->getListView()->setSelectedItem(index);

				// Open the menu normally
				addPopupWidget(rightClickMenuPopupSingleSelection, mousePos.x, mousePos.y, 150, rightClickMenuPopupSingleSelection->getSize().y);
			}
		});
	}
	listView->getListView()->connect("DoubleClicked", [this, listView, openLevelPackObjectTab](int index) {
		openLevelPackObjectTab(listView->getLevelPackObjectIDFromIndex(index));
	});
	listPanel->add(listView);
}

void MainEditorWindow::openLeftPanelAttack(int attackID) {
	// Open attacks tab in left panel if not already open
	if (leftPanel->getSelectedTab() != LEFT_PANEL_ATTACK_LIST_TAB_NAME) {
		leftPanel->selectTab(LEFT_PANEL_ATTACK_LIST_TAB_NAME);
	}
	// Select the attack in attacksListView
	attacksListView->getListView()->setSelectedItem(attacksListView->getIndexFromLevelPackObjectID(attackID));

	// Get the attack
	std::shared_ptr<EditorAttack> openedAttack;
	if (unsavedAttacks.count(attackID) > 0) {
		// There are unsaved changes for this attack, so open the one with unsaved changes
		openedAttack = std::dynamic_pointer_cast<EditorAttack>(unsavedAttacks[attackID]);
	} else {
		// Make a copy of the attack in the LevelPack so that changes can be applied/discarded
		// whenever the user wants instead of modifying the LevelPack directly.
		openedAttack = std::make_shared<EditorAttack>(levelPack->getAttack(attackID));
	}

	// Open the tab in mainPanel
	if (mainPanel->hasTab(format(MAIN_PANEL_ATTACK_TAB_NAME_FORMAT, attackID))) {
		// Tab already exists, so just select it
		mainPanel->selectTab(format(MAIN_PANEL_ATTACK_TAB_NAME_FORMAT, attackID));
	} else {
		// Create the tab
		std::shared_ptr<AttackEditorPanel> attackEditorPanel = AttackEditorPanel::create(*this, levelPack, *spriteLoader, clipboard, openedAttack);
		attackEditorPanel->connect("AttackPatternBeginEdit", [this](int attackPatternID) {
			openLeftPanelAttackPattern(attackPatternID);
		});
		attackEditorPanel->connect("AttackModified", [this](std::shared_ptr<EditorAttack> attack) {
			unsavedAttacks[attack->getID()] = std::dynamic_pointer_cast<LevelPackObject>(attack);
			attacksListView->reload();

			previewWindow->onOriginalLevelPackAttackModified(attack);
		});
		std::string tabName = format(MAIN_PANEL_ATTACK_TAB_NAME_FORMAT, attackID);
		mainPanel->addTab(tabName, attackEditorPanel, true, true);
		mainPanel->setTabOnSelectFunction(tabName, [this, openedAttack]() {
			previewWindow->previewAttack(openedAttack);
		});
	}

	// View preview
	previewWindow->previewAttack(openedAttack);
}

void MainEditorWindow::openLeftPanelAttackPattern(int id) {
	// Open tab in left panel if not already open
	if (leftPanel->getSelectedTab() != LEFT_PANEL_ATTACK_PATTERN_LIST_TAB_NAME) {
		leftPanel->selectTab(LEFT_PANEL_ATTACK_PATTERN_LIST_TAB_NAME);
	}
	// Select the object in the list view
	attackPatternsListView->getListView()->setSelectedItem(attackPatternsListView->getIndexFromLevelPackObjectID(id));

	// Get the object
	std::shared_ptr<EditorAttackPattern> openedObject;
	if (unsavedAttacks.count(id) > 0) {
		// There are unsaved changes for this object, so open the one with unsaved changes
		openedObject = std::dynamic_pointer_cast<EditorAttackPattern>(unsavedAttackPatterns[id]);
	} else {
		// Make a copy of the object in the LevelPack so that changes can be applied/discarded
		// whenever the user wants instead of modifying the LevelPack directly.
		openedObject = std::make_shared<EditorAttackPattern>(levelPack->getAttackPattern(id));
	}

	// Open the tab in mainPanel
	if (mainPanel->hasTab(format(MAIN_PANEL_ATTACK_PATTERN_TAB_NAME_FORMAT, id))) {
		// Tab already exists, so just select it
		mainPanel->selectTab(format(MAIN_PANEL_ATTACK_PATTERN_TAB_NAME_FORMAT, id));
	} else {
		// Create the tab
		std::shared_ptr<AttackPatternEditorPanel> attackPatternEditorPanel = AttackPatternEditorPanel::create(*this, levelPack, *spriteLoader, clipboard, openedObject);
		attackPatternEditorPanel->connect("EnemyPhaseBeginEdit", [this](int enemyPhaseID) {
			openLeftPanelEnemyPhase(enemyPhaseID);
		});
		attackPatternEditorPanel->connect("PlayerBeginEdit", [this]() {
			openLeftPanelPlayer();
		});
		attackPatternEditorPanel->connect("AttackPatternModified", [this](std::shared_ptr<EditorAttackPattern> attackPattern) {
			unsavedAttacks[attackPattern->getID()] = std::dynamic_pointer_cast<LevelPackObject>(attackPattern);
			attackPatternsListView->reload();

			previewWindow->onOriginalLevelPackAttackPatternModified(attackPattern);
		});
		std::string tabName = format(MAIN_PANEL_ATTACK_PATTERN_TAB_NAME_FORMAT, id);
		mainPanel->addTab(tabName, attackPatternEditorPanel, true, true);
		mainPanel->setTabOnSelectFunction(tabName, [this, openedObject]() {
			previewWindow->previewAttackPattern(openedObject);
		});
	}

	// View preview
	previewWindow->previewAttackPattern(openedObject);
}

void MainEditorWindow::overwriteAttackPatterns(std::vector<std::shared_ptr<EditorAttackPattern>> objects, UndoStack* undoStack) {
	std::vector<std::shared_ptr<EditorAttackPattern>> oldObjects;
	for (std::shared_ptr<EditorAttackPattern> obj : objects) {
		int id = obj->getID();
		if (unsavedAttackPatterns.count(id) > 0) {
			oldObjects.push_back(std::dynamic_pointer_cast<EditorAttackPattern>(unsavedAttackPatterns[id]->clone()));
		} else {
			oldObjects.push_back(std::make_shared<EditorAttackPattern>(levelPack->getAttackPattern(id)));
		}
	}

	if (undoStack) {
		undoStack->execute(UndoableCommand([this, objects]() {
			for (std::shared_ptr<EditorAttackPattern> obj : objects) {
				int id = obj->getID();

				// Overwrite existing object
				if (unsavedAttackPatterns.count(id) > 0) {
					std::dynamic_pointer_cast<EditorAttackPattern>(unsavedAttackPatterns[id])->load(obj->format());
				} else {
					unsavedAttackPatterns[id] = std::dynamic_pointer_cast<LevelPackObject>(std::make_shared<EditorAttackPattern>(obj));
				}

				reloadAttackPatternTab(id);
				attackPatternsListView->reload();
			}
		}, [this, oldObjects]() {
			for (std::shared_ptr<EditorAttackPattern> obj : oldObjects) {
				int id = obj->getID();

				// Overwrite existing object
				if (unsavedAttackPatterns.count(id) > 0) {
					std::dynamic_pointer_cast<EditorAttackPattern>(unsavedAttackPatterns[id])->load(obj->format());
				} else {
					unsavedAttackPatterns[id] = std::dynamic_pointer_cast<LevelPackObject>(std::make_shared<EditorAttackPattern>(obj));
				}

				reloadAttackPatternTab(id);
				attackPatternsListView->reload();
			}
		}));
	} else {
		// Overwrite without pushing to an undo stack

		for (std::shared_ptr<EditorAttackPattern> obj : objects) {
			int id = obj->getID();

			// Overwrite existing object
			if (unsavedAttackPatterns.count(id) > 0) {
				std::dynamic_pointer_cast<EditorAttackPattern>(unsavedAttackPatterns[id])->load(obj->format());
			} else {
				unsavedAttackPatterns[id] = std::dynamic_pointer_cast<LevelPackObject>(std::make_shared<EditorAttackPattern>(obj));
			}

			reloadAttackPatternTab(id);
			attackPatternsListView->reload();
		}
	}
}

void MainEditorWindow::reloadAttackTab(int attackID) {
	std::string tabName = format(MAIN_PANEL_ATTACK_TAB_NAME_FORMAT, attackID);
	// Do nothing if the tab doesn't exist
	if (mainPanel->hasTab(tabName)) {
		// Remove the tab and then re-insert it at its old index
		bool tabWasSelected = mainPanel->getSelectedTab() == tabName;
		int tabIndex = mainPanel->getTabIndex(tabName);
		mainPanel->removeTab(tabName);

		// Get the attack
		std::shared_ptr<EditorAttack> openedAttack;
		if (unsavedAttacks.count(attackID) > 0) {
			// There are unsaved changes for this attack, so open the one with unsaved changes
			openedAttack = std::dynamic_pointer_cast<EditorAttack>(unsavedAttacks[attackID]);
		} else if (levelPack->hasAttack(attackID)) {
			// Make a copy of the attack in the LevelPack so that changes can be applied/discarded
			// whenever the user wants instead of modifying the LevelPack directly.
			openedAttack = std::make_shared<EditorAttack>(levelPack->getAttack(attackID));
		} else {
			// The attack doesn't exist, so keep the tab removed
			return;
		}

		// Create the tab
		std::shared_ptr<AttackEditorPanel> attackEditorPanel = AttackEditorPanel::create(*this, levelPack, *spriteLoader, clipboard, openedAttack);
		attackEditorPanel->connect("AttackPatternBeginEdit", [this](int attackPatternID) {
			openLeftPanelAttackPattern(attackPatternID);
		});
		attackEditorPanel->connect("AttackModified", [this](std::shared_ptr<EditorAttack> attack) {
			unsavedAttacks[attack->getID()] = std::dynamic_pointer_cast<LevelPackObject>(attack);
			attacksListView->reload();
		});
		std::string tabName = format(MAIN_PANEL_ATTACK_TAB_NAME_FORMAT, attackID);
		mainPanel->insertTab(tabName, attackEditorPanel, tabIndex, tabWasSelected, true);
		mainPanel->setTabOnSelectFunction(tabName, [this, openedAttack]() {
			previewWindow->previewAttack(openedAttack);
		});
	}
}

void MainEditorWindow::reloadAttackPatternTab(int id) {
	std::string tabName = format(MAIN_PANEL_ATTACK_PATTERN_TAB_NAME_FORMAT, id);
	// Do nothing if the tab doesn't exist
	if (mainPanel->hasTab(tabName)) {
		// Remove the tab and then re-insert it at its old index
		bool tabWasSelected = mainPanel->getSelectedTab() == tabName;
		int tabIndex = mainPanel->getTabIndex(tabName);
		mainPanel->removeTab(tabName);

		// Get the object
		std::shared_ptr<EditorAttackPattern> openedObject;
		if (unsavedAttackPatterns.count(id) > 0) {
			// There are unsaved changes for this attack, so open the one with unsaved changes
			openedObject = std::dynamic_pointer_cast<EditorAttackPattern>(unsavedAttackPatterns[id]);
		} else if (levelPack->hasAttackPattern(id)) {
			// Make a copy of the attack in the LevelPack so that changes can be applied/discarded
			// whenever the user wants instead of modifying the LevelPack directly.
			openedObject = std::make_shared<EditorAttackPattern>(levelPack->getAttackPattern(id));
		} else {
			// The object doesn't exist, so keep the tab removed
			return;
		}

		// Create the tab
		std::shared_ptr<AttackPatternEditorPanel> attackPatternEditorPanel = AttackPatternEditorPanel::create(*this, levelPack, *spriteLoader, clipboard, openedObject);
		attackPatternEditorPanel->connect("EnemyPhaseBeginEdit", [this](int enemyPhaseID) {
			openLeftPanelEnemyPhase(enemyPhaseID);
		});
		attackPatternEditorPanel->connect("PlayerBeginEdit", [this]() {
			openLeftPanelPlayer();
		});
		attackPatternEditorPanel->connect("AttackPatternModified", [this](std::shared_ptr<EditorAttackPattern> attackPattern) {
			unsavedAttacks[attackPattern->getID()] = std::dynamic_pointer_cast<LevelPackObject>(attackPattern);
			attackPatternsListView->reload();

			previewWindow->onOriginalLevelPackAttackPatternModified(attackPattern);
		});
		std::string tabName = format(MAIN_PANEL_ATTACK_PATTERN_TAB_NAME_FORMAT, id);
		mainPanel->insertTab(tabName, attackPatternEditorPanel, tabIndex, tabWasSelected, true);
		mainPanel->setTabOnSelectFunction(tabName, [this, openedObject]() {
			previewWindow->previewAttackPattern(openedObject);
		});
	}
}

std::shared_ptr<LevelPackObjectsListView> MainEditorWindow::getAttacksListView() {
	return attacksListView;
}

std::shared_ptr<LevelPackObjectsListPanel> MainEditorWindow::getAttacksListPanel() {
	return attacksListPanel;
}

std::map<int, std::shared_ptr<LevelPackObject>>& MainEditorWindow::getUnsavedAttacks() {
	return unsavedAttacks;
}

std::map<int, std::shared_ptr<LevelPackObject>>& MainEditorWindow::getUnsavedAttackPatterns() {
	return unsavedAttackPatterns;
}

std::map<int, std::shared_ptr<LevelPackObject>>& MainEditorWindow::getUnsavedEnemies() {
	return unsavedEnemies;
}

std::map<int, std::shared_ptr<LevelPackObject>>& MainEditorWindow::getUnsavedEnemyPhases() {
	return unsavedEnemyPhases;
}

std::map<int, std::shared_ptr<LevelPackObject>>& MainEditorWindow::getUnsavedBulletModels() {
	return unsavedBulletModels;
}

void MainEditorWindow::createAttack() {
	int id = levelPack->getNextAttackID();
	attacksListView->getUndoStack().execute(UndoableCommand([this, id]() {
		levelPack->createAttack(id);
		
		// Open the newly created attack
		openLeftPanelAttack(id);

		attacksListView->reload();

		// Select it in attacksListView
		attacksListView->getListView()->setSelectedItem(attacksListView->getIndexFromLevelPackObjectID(id));
	}, [this, id]() {
		levelPack->deleteAttack(id);
		attacksListView->reload();
	}));
}

void MainEditorWindow::createAttackPattern() {
	int id = levelPack->getNextAttackPatternID();
	attackPatternsListView->getUndoStack().execute(UndoableCommand([this, id]() {
		levelPack->createAttackPattern(id);

		// Open the newly created attack
		openLeftPanelAttackPattern(id);

		attackPatternsListView->reload();

		// Select it in attacksListView
		attackPatternsListView->getListView()->setSelectedItem(attackPatternsListView->getIndexFromLevelPackObjectID(id));
	}, [this, id]() {
		levelPack->deleteAttackPattern(id);
		attackPatternsListView->reload();
	}));
}

LevelPackObjectPreviewWindow::LevelPackObjectPreviewWindow(std::shared_ptr<std::recursive_mutex> tguiMutex, std::string windowTitle, int width, int height, std::string levelPackName, bool scaleWidgetsOnResize, bool letterboxingEnabled, float renderInterval)
	: EditorWindow(tguiMutex, windowTitle, width, height, scaleWidgetsOnResize, letterboxingEnabled, renderInterval), levelPackName(levelPackName) {
}

void LevelPackObjectPreviewWindow::previewNothing() {
	if (window->isOpen()) {
		previewObjectLabel->setText("Nothing being previewed");

		previewThread.join();
		previewThread = boost::thread(&LevelPackObjectPreviewPanel::previewNothing, &(*previewPanel));
		previewThread.detach();
	}
}

void LevelPackObjectPreviewWindow::previewAttack(const std::shared_ptr<EditorAttack> attack) {
	if (window->isOpen() && !lockCurrentPreview) {
		previewObjectLabel->setText("Attack ID " + std::to_string(attack->getID()));

		previewThread.join();
		previewThread = boost::thread(&LevelPackObjectPreviewPanel::previewAttack, &(*previewPanel), attack);
		previewThread.detach();
	}
}

void LevelPackObjectPreviewWindow::previewAttackPattern(const std::shared_ptr<EditorAttackPattern> attackPattern) {
	if (window->isOpen() && !lockCurrentPreview) {
		previewObjectLabel->setText("Attack Pattern ID " + std::to_string(attackPattern->getID()));

		previewThread.join();
		previewThread = boost::thread(&LevelPackObjectPreviewPanel::previewAttackPattern, &(*previewPanel), attackPattern);
		previewThread.detach();
	}
}

void LevelPackObjectPreviewWindow::onOriginalLevelPackAttackModified(const std::shared_ptr<EditorAttack> attack) {
	if (window->isOpen()) {
		previewPanel->getLevelPack()->updateAttack(attack);
	}
}

void LevelPackObjectPreviewWindow::onOriginalLevelPackAttackPatternModified(const std::shared_ptr<EditorAttackPattern> attackPattern) {
	if (window->isOpen()) {
		previewPanel->getLevelPack()->updateAttackPattern(attackPattern);
	}
}

void LevelPackObjectPreviewWindow::updateWidgetsPositionsAndSizes() {
	if (infoPanel->isVisible()) {
		infoPanel->setSize("20%", "100%");
		if (logs->isVisible()) {
			previewPanel->setSize("80%", "60%");
		} else {
			previewPanel->setSize("80%", "100%");
		}
		previewPanel->setPosition("20%", 0);
	} else {
		if (logs->isVisible()) {
			previewPanel->setSize("100%", "60%");
		} else {
			previewPanel->setSize("100%", "100%");
		}
		previewPanel->setPosition(0, 0);
	}
}

bool LevelPackObjectPreviewWindow::handleEvent(sf::Event event) {
	if (EditorWindow::handleEvent(event)) {
		return true;
	} else if (previewPanel->handleEvent(event)) {
		return true;
	} else if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::I) {
			infoPanel->setVisible(!infoPanel->isVisible());
			updateWidgetsPositionsAndSizes();
			return true;
		} else if (event.key.code == sf::Keyboard::L) {
			logs->setVisible(!logs->isVisible());
			updateWidgetsPositionsAndSizes();
			return true;
		} else if (event.key.code == sf::Keyboard::T) {
			previewPanel->setUseDebugRenderSystem(!useDebugRenderSystem);
			ignoreSignals = true;
			useDebugRenderSystem->setChecked(previewPanel->getUseDebugRenderSystem());
			ignoreSignals = false;

			return true;
		} else if (event.key.code == sf::Keyboard::G) {
			previewPanel->resetCamera();
			return true;
		} else if (event.key.code == sf::Keyboard::R) {
			previewPanel->resetPreview();
			return true;
		} else if (event.key.code == sf::Keyboard::Escape) {
			previewNothing();
			return true;
		}
	}
	return previewPanel->handleEvent(event);
}

void LevelPackObjectPreviewWindow::onRenderWindowInitialization() {
	previewPanel = LevelPackObjectPreviewPanel::create(*this, levelPackName);
	previewPanel->setSize("100%", "100%");
	gui->add(previewPanel);

	infoPanel = tgui::Panel::create();
	infoPanel->setVisible(false);
	gui->add(infoPanel);

	previewObjectLabel = tgui::Label::create();
	delayLabel = tgui::Label::create();
	delay = NumericalEditBoxWithLimits::create();
	timeMultiplierLabel = tgui::Label::create();
	timeMultiplier = SliderWithEditBox::create(false);
	resetPreview = tgui::Button::create();
	resetCamera = tgui::Button::create();
	setPlayerSpawn = tgui::Button::create();
	setSource = tgui::Button::create();
	useDebugRenderSystem = tgui::CheckBox::create();
	lockCurrentPreviewCheckBox = tgui::CheckBox::create();

	delay->setMin(0);
	delay->setValue(previewPanel->getAttackLoopDelay());
	timeMultiplier->setMin(0, false);
	timeMultiplier->setMax(1, false);
	timeMultiplier->setStep(0.01f);
	timeMultiplier->setValue(previewPanel->getTimeMultiplier());
	useDebugRenderSystem->setChecked(previewPanel->getUseDebugRenderSystem());
	lockCurrentPreviewCheckBox->setChecked(lockCurrentPreview);

	delay->connect("ValueChanged", [this](float value) {
		if (ignoreSignals) {
			return;
		}

		previewPanel->setAttackLoopDelay(value);
	});
	timeMultiplier->connect("ValueChanged", [this](float value) {
		if (ignoreSignals) {
			return;
		}

		previewPanel->setTimeMultiplier(value);
	});
	resetPreview->connect("Pressed", [this]() {
		previewPanel->resetPreview();
	});
	resetCamera->connect("Pressed", [this]() {
		previewPanel->resetCamera();
	});
	setPlayerSpawn->connect("Pressed", [this]() {
		previewPanel->setSettingPlayerSpawn(true);
	});
	setSource->connect("Pressed", [this]() {
		previewPanel->setSettingSource(true);
	});
	useDebugRenderSystem->connect("Changed", [this](bool checked) {
		if (ignoreSignals) {
			return;
		}

		previewPanel->setUseDebugRenderSystem(checked);
	});
	lockCurrentPreviewCheckBox->connect("Changed", [this](bool checked) {
		if (ignoreSignals) {
			return;
		}

		this->lockCurrentPreview = checked;
	});

	useDebugRenderSystem->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	lockCurrentPreviewCheckBox->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);

	previewObjectLabel->setTextSize(TEXT_SIZE);
	delayLabel->setTextSize(TEXT_SIZE);
	delay->setTextSize(TEXT_SIZE);
	timeMultiplierLabel->setTextSize(TEXT_SIZE);
	timeMultiplier->setTextSize(TEXT_SIZE);
	resetPreview->setTextSize(TEXT_SIZE);
	resetCamera->setTextSize(TEXT_SIZE);
	setPlayerSpawn->setTextSize(TEXT_SIZE);
	setSource->setTextSize(TEXT_SIZE);
	useDebugRenderSystem->setTextSize(TEXT_SIZE);
	lockCurrentPreviewCheckBox->setTextSize(TEXT_SIZE);

	delayLabel->setText("Attack/Attack pattern repeat interval");
	timeMultiplierLabel->setText("Time multiplier");
	resetPreview->setText("Reset preview");
	resetCamera->setText("Reset camera");
	setPlayerSpawn->setText("Set player spawn");
	setSource->setText("Set preview source");
	useDebugRenderSystem->setText("Hitboxes visible");
	lockCurrentPreviewCheckBox->setText("Lock current preview");

	delayLabel->setToolTip(createToolTip("Seconds before the attack or attack pattern being previewed is executed again."));
	useDebugRenderSystem->setToolTip(createToolTip("If this is checked, all hitboxes will be visible and sprites will be drawn outside the map boundaries \
but shader effects (such as piercing bullets flashing after hitting a player) will be unavailable due to technical limitations. Note that bullets with collision action \
\"Destroy self only\" only become invisible after hitting a player, so their hitboxes will still be visible even though they cannot interact with the player again."));
	lockCurrentPreviewCheckBox->setToolTip(createToolTip("While this is checked, new objects will not be previewed."));
	setPlayerSpawn->setToolTip(createToolTip("Sets the player's spawn position for future previews to be the next clicked position. The spawn position must be within the map boundaries."));
	setSource->setToolTip(createToolTip("Sets the preview source position for future previews to be the next clicked position. The spawn position can be outside the map boundaries."));

	previewObjectLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	delayLabel->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(previewObjectLabel) + GUI_PADDING_Y);
	delay->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(delayLabel) + GUI_LABEL_PADDING_Y);
	timeMultiplierLabel->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(delay) + GUI_PADDING_Y);
	timeMultiplier->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(timeMultiplierLabel) + GUI_LABEL_PADDING_Y);
	resetPreview->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(timeMultiplier) + GUI_PADDING_Y);
	resetCamera->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(resetPreview) + GUI_PADDING_Y);
	setPlayerSpawn->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(resetCamera) + GUI_PADDING_Y);
	setSource->setPosition(tgui::bindRight(setPlayerSpawn) + GUI_PADDING_X, tgui::bindTop(setPlayerSpawn));
	useDebugRenderSystem->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(setSource) + GUI_PADDING_Y);
	lockCurrentPreviewCheckBox->setPosition(tgui::bindLeft(previewObjectLabel), tgui::bindBottom(useDebugRenderSystem) + GUI_PADDING_Y);

	infoPanel->connect("SizeChanged", [this](sf::Vector2f newSize) {
		delay->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
		timeMultiplier->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
		resetPreview->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BUTTON_HEIGHT);
		resetCamera->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BUTTON_HEIGHT);
		setPlayerSpawn->setSize((newSize.x - GUI_PADDING_X * 3)/2.0f, TEXT_BUTTON_HEIGHT);
		setSource->setSize((newSize.x - GUI_PADDING_X * 3) / 2.0f, TEXT_BUTTON_HEIGHT);
	});
	infoPanel->setSize("30%", "100%");

	infoPanel->add(previewObjectLabel);
	infoPanel->add(delayLabel);
	infoPanel->add(delay);
	infoPanel->add(timeMultiplierLabel);
	infoPanel->add(timeMultiplier);
	infoPanel->add(resetPreview);
	infoPanel->add(resetCamera);
	infoPanel->add(setPlayerSpawn);
	infoPanel->add(setSource);
	infoPanel->add(useDebugRenderSystem);
	infoPanel->add(lockCurrentPreviewCheckBox);

	logs = tgui::TextBox::create();
	logs->setVisible(false);
	logs->setSize(tgui::bindWidth(previewPanel), "40%");
	logs->setEnabled(false);
	logs->setPosition(tgui::bindLeft(previewPanel), tgui::bindBottom(previewPanel));
	gui->add(logs);

	previewPanel->connect("PreviewAttempted", [this](LevelPackObject::LEGAL_STATUS status, std::vector<std::string> messages) {
		// The first string in messages is a description of the attempted action

		if (status == LevelPackObject::LEGAL_STATUS::LEGAL) {
			logs->setText(messages[0]);
		} else if (status == LevelPackObject::LEGAL_STATUS::WARNING) {
			std::string fullString = messages[0] + "\nWarning(s):";
			for (int i = 1; i < messages.size(); i++) {
				fullString += "\n" + messages[i];
			}
			logs->setText(fullString);
		} else {
			std::string fullString = messages[0] + "\nError(s):";
			for (int i = 1; i < messages.size(); i++) {
				fullString += "\n" + messages[i];
			}
			logs->setText(fullString);
		}
	});

	previewNothing();
	updateWidgetsPositionsAndSizes();
}

void LevelPackObjectPreviewWindow::physicsUpdate(float deltaTime) {
	EditorWindow::physicsUpdate(deltaTime);
	previewPanel->physicsUpdate(deltaTime);
}

void LevelPackObjectPreviewWindow::render(float deltaTime) {
	previewPanel->renderUpdate(deltaTime);
	EditorWindow::render(deltaTime);
}
