#include "EditorWindow.h"
#include "Components.h"
#include "Enemy.h"
#include "EnemyPhase.h"
#include "Level.h"
#include "AttackEditorPanel.h"
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
	if (window && window->isOpen()) {
		window->close();
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

	confirmationYes->connect("Pressed", [&]() {
		confirmationSignal->publish(true);
		confirmationSignal->sink().disconnect();
		closeConfirmationPanel();
	});
	confirmationNo->connect("Pressed", [&]() {
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

void EditorWindow::closeConfirmationPanel() {
	gui->remove(confirmationPanel);
	confirmationPanelOpen = false;

	for (auto ptr : widgetsToBeEnabledAfterConfirmationPrompt) {
		ptr->setEnabled(true);
	}
	widgetsToBeEnabledAfterConfirmationPrompt.clear();
}

AttacksListPanel::AttacksListPanel(MainEditorWindow& mainEditorWindow, Clipboard& clipboard) : mainEditorWindow(mainEditorWindow), clipboard(clipboard) {

}

bool AttacksListPanel::handleEvent(sf::Event event) {
	if (mainEditorWindow.getAttacksListView()->handleEvent(event)) {
		return true;
	}
	return false;
}

void AttacksListPanel::setLevelPack(LevelPack* levelPack) {
	this->levelPack = levelPack;
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
		attacksListPanel = AttacksListPanel::create(*this, clipboard);
		{
			// Add button
			auto attacksAddButton = tgui::Button::create();
			attacksAddButton->setText("+");
			attacksAddButton->setPosition(0, 0);
			attacksAddButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
			attacksAddButton->connect("Pressed", [this]() {
				createAttack();
			});
			attacksListPanel->add(attacksAddButton);

			// Save all button
			auto attacksSaveAllButton = tgui::Button::create();
			attacksSaveAllButton->setText("S");
			attacksSaveAllButton->setPosition(tgui::bindRight(attacksAddButton), 0);
			attacksSaveAllButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
			attacksSaveAllButton->connect("Pressed", [this]() {
				attacksListView->manualSaveAll();
			});
			attacksListPanel->add(attacksSaveAllButton);

			// Sort button
			auto attacksSortButton = tgui::Button::create();
			attacksSortButton->setText("=");
			attacksSortButton->setPosition(tgui::bindRight(attacksSaveAllButton), 0);
			attacksSortButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
			attacksSortButton->connect("Pressed", [this]() {
				attacksListView->cycleSortOption();
			});
			attacksListPanel->add(attacksSortButton);

			// List view
			attacksListView->setPosition(0, tgui::bindBottom(attacksAddButton));
			attacksListView->setSize("100%", tgui::bindHeight(attacksListPanel) - tgui::bindBottom(attacksAddButton));
			{
				// Right click menu
				// Menu for single attack selection
				auto rightClickMenuPopupSingleSelection = createMenuPopup({
					std::make_pair("Open", [&]() {
						openLeftPanelAttack(attacksListView->getAttackIDFromIndex(attacksListView->getListView()->getSelectedItemIndex()));
					}),
					std::make_pair("Copy", [&]() {
						attacksListView->manualCopy();
					}),
					std::make_pair("Paste", [&]() {
						attacksListView->manualPaste();
					}),
					std::make_pair("Paste (override this)", [&]() {
						attacksListView->manualPaste2();
					}),
					std::make_pair("Save", [&]() {
						attacksListView->manualSave();
					}),
					std::make_pair("Delete", [&]() {
						attacksListView->manualDelete();
					})
				});
				// Menu for multiple attack selections
				auto rightClickMenuPopupMultiSelection = createMenuPopup({
					std::make_pair("Copy", [&]() {
						attacksListView->manualCopy();
					}),
					std::make_pair("Paste", [&]() {
						attacksListView->manualPaste();
					}),
					std::make_pair("Paste (override these)", [&]() {
						attacksListView->manualPaste2();
					}),
					std::make_pair("Save", [&]() {
						attacksListView->manualSave();
					}),
					std::make_pair("Delete", [&]() {
						attacksListView->manualDelete();
					})
				});
				attacksListView->getListView()->connect("RightClicked", [&, rightClickMenuPopupSingleSelection, rightClickMenuPopupMultiSelection](int index) {
					std::set<std::size_t> selectedItemIndices = attacksListView->getListView()->getSelectedItemIndices();
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
						attacksListView->getListView()->setSelectedItem(index);

						// Open the menu normally
						addPopupWidget(rightClickMenuPopupSingleSelection, mousePos.x, mousePos.y, 150, rightClickMenuPopupSingleSelection->getSize().y);
					}
				});
			}
			attacksListView->getListView()->connect("DoubleClicked", [&](int index) {
				openLeftPanelAttack(attacksListView->getAttackIDFromIndex(index));
			});
			attacksListPanel->add(attacksListView);
		}
		leftPanel->addTab(LEFT_PANEL_ATTACK_LIST_TAB_NAME, attacksListPanel, true);
	}

	mainPanel = TabsWithPanel::create(*this);
	mainPanel->setPosition(tgui::bindRight(leftPanel), tgui::bindTop(leftPanel));
	mainPanel->setSize("80%", "100%");
	mainPanel->setVisible(true);
	mainPanel->setMoreTabsListAlignment(TabsWithPanel::MoreTabsListAlignment::Left);
	gui->add(mainPanel);
}

void MainEditorWindow::loadLevelPack(std::string levelPackName) {
	audioPlayer = std::make_shared<AudioPlayer>();
	levelPack = std::make_shared<LevelPack>(*audioPlayer, levelPackName);
	spriteLoader = levelPack->createSpriteLoader();
	spriteLoader->preloadTextures();

	attacksListPanel->setLevelPack(levelPack.get());
	attacksListView->setLevelPack(levelPack.get());
	attacksListView->reload();
}

void MainEditorWindow::overwriteAttacks(std::vector<std::shared_ptr<EditorAttack>> attacks, UndoStack* undoStack) {
	std::vector<std::shared_ptr<EditorAttack>> oldAttacks;
	for (std::shared_ptr<EditorAttack> attack : attacks) {
		int id = attack->getID();
		if (unsavedAttacks.count(id) > 0) {
			oldAttacks.push_back(std::make_shared<EditorAttack>(unsavedAttacks[id]));
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
					unsavedAttacks[id]->load(attack->format());
				} else {
					unsavedAttacks[id] = std::make_shared<EditorAttack>(attack);
				}

				reloadAttackTab(id);
				attacksListView->reload();
			}
		}, [this, oldAttacks]() {
			for (std::shared_ptr<EditorAttack> attack : oldAttacks) {
				int id = attack->getID();

				// Overwrite existing EditorAttack
				if (unsavedAttacks.count(id) > 0) {
					unsavedAttacks[id]->load(attack->format());
				} else {
					unsavedAttacks[id] = std::make_shared<EditorAttack>(attack);
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
				unsavedAttacks[id]->load(attack->format());
			} else {
				unsavedAttacks[id] = std::make_shared<EditorAttack>(attack);
			}

			reloadAttackTab(id);
			attacksListView->reload();
		}
	}
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

void MainEditorWindow::openLeftPanelAttack(int attackID) {
	// Open attacks tab in left panel if not already open
	if (leftPanel->getSelectedTab() != LEFT_PANEL_ATTACK_LIST_TAB_NAME) {
		leftPanel->selectTab(LEFT_PANEL_ATTACK_LIST_TAB_NAME);
	}
	// Select the attack in attacksListView
	attacksListView->getListView()->setSelectedItem(attacksListView->getIndexFromAttackID(attackID));

	// Get the attack
	std::shared_ptr<EditorAttack> openedAttack;
	if (unsavedAttacks.count(attackID) > 0) {
		// There are unsaved changes for this attack, so open the one with unsaved changes
		openedAttack = unsavedAttacks[attackID];
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
		std::shared_ptr<AttackEditorPanel> attackEditorPanel = AttackEditorPanel::create(*this, *levelPack, *spriteLoader, clipboard, openedAttack);
		attackEditorPanel->connect("AttackPatternBeginEdit", [&](int attackPatternID) {
			openLeftPanelAttackPattern(attackPatternID);
		});
		attackEditorPanel->connect("AttackModified", [&](std::shared_ptr<EditorAttack> attack) {
			unsavedAttacks[attack->getID()] = attack;
			attacksListView->reload();
		});
		mainPanel->addTab(format(MAIN_PANEL_ATTACK_TAB_NAME_FORMAT, attackID), attackEditorPanel, true, true);
	}

	//TODO: load attack in preview panel; AttackModified -> update it
}

void MainEditorWindow::openLeftPanelAttackPattern(int attackPatternID) {
	//TODO
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
			openedAttack = unsavedAttacks[attackID];
		} else if (levelPack->hasAttack(attackID)) {
			// Make a copy of the attack in the LevelPack so that changes can be applied/discarded
			// whenever the user wants instead of modifying the LevelPack directly.
			openedAttack = std::make_shared<EditorAttack>(levelPack->getAttack(attackID));
		} else {
			// The attack doesn't exist, so keep the tab removed
			return;
		}

		// Create the tab
		std::shared_ptr<AttackEditorPanel> attackEditorPanel = AttackEditorPanel::create(*this, *levelPack, *spriteLoader, clipboard, openedAttack);
		attackEditorPanel->connect("AttackPatternBeginEdit", [&](int attackPatternID) {
			openLeftPanelAttackPattern(attackPatternID);
		});
		attackEditorPanel->connect("AttackModified", [&](std::shared_ptr<EditorAttack> attack) {
			unsavedAttacks[attack->getID()] = attack;
			attacksListView->reload();
		});
		mainPanel->insertTab(format(MAIN_PANEL_ATTACK_TAB_NAME_FORMAT, attackID), attackEditorPanel, tabIndex, tabWasSelected, true);
	}
}

std::shared_ptr<AttacksListView> MainEditorWindow::getAttacksListView() {
	return attacksListView;
}

std::shared_ptr<AttacksListPanel> MainEditorWindow::getAttacksListPanel() {
	return attacksListPanel;
}

std::map<int, std::shared_ptr<EditorAttack>>& MainEditorWindow::getUnsavedAttacks() {
	return unsavedAttacks;
}

void MainEditorWindow::createAttack() {
	int id = levelPack->getNextAttackID();
	attacksListView->getUndoStack().execute(UndoableCommand([this, id]() {
		levelPack->createAttack(id);
		
		// Open the newly created attack
		openLeftPanelAttack(id);

		attacksListView->reload();

		// Select it in attacksListView
		attacksListView->getListView()->setSelectedItem(attacksListView->getIndexFromAttackID(id));
	}, [this, id]() {
		levelPack->deleteAttack(id);
		attacksListView->reload();
	}));
}