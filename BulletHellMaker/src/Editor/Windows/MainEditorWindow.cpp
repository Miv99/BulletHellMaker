#include <Editor/Windows/MainEditorWindow.h>

#include <Mutex.h>
#include <Constants.h>
#include <GuiConfig.h>
#include <Editor/Util/EditorUtils.h>
#include <Editor/Attack/AttackEditorPanel.h>
#include <Editor/AttackPattern/AttackPatternEditorPanel.h>
#include <Editor/LevelPackObjectList/LevelPackObjectsListView.h>
#include <Editor/LevelPackObjectList/LevelPackObjectsListPanel.h>
#include <Editor/Windows/LevelPackObjectPreviewWindow.h>
#include <Editor/Windows/MainEditorWindowMenuBar.h>
#include <Game/EntityCreationQueue.h>

MainEditorWindow::MainEditorWindow(std::string windowTitle, int width, int height, bool scaleWidgetsOnResize, bool letterboxingEnabled, float renderInterval)
	: EditorWindow(windowTitle, width, height, scaleWidgetsOnResize, letterboxingEnabled, renderInterval) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	std::shared_ptr<MainEditorWindowMenuBar> menuBar = MainEditorWindowMenuBar::create(*this);
	menuBar->setTextSize(TEXT_SIZE);
	menuBar->setSize("100%", TEXT_BUTTON_HEIGHT);
	
	leftPanel = TabsWithPanel::create(*this);
	leftPanel->setPosition(0, tgui::bindBottom(menuBar));
	leftPanel->setSize("20%", "100%");
	leftPanel->setVisible(true);
	leftPanel->setMoreTabsListAlignment(TabsWithPanel::MoreTabsListAlignment::Right);
	gui->add(leftPanel);

	{
		// Attacks tab in left panel
		attacksListView = AttacksListView::create(*this, clipboard);
		attacksListPanel = LevelPackObjectsListPanel::create(*attacksListView);
		populateLeftPanelLevelPackObjectListPanel(attacksListView, attacksListPanel,
			[this]() {
			this->createAttack(true);
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
		attackPatternsListPanel = LevelPackObjectsListPanel::create(*attackPatternsListView);
		populateLeftPanelLevelPackObjectListPanel(attackPatternsListView, attackPatternsListPanel,
			[this]() {
			this->createAttackPattern(true);
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

	gui->add(menuBar);

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
	try {
		levelPack = std::make_shared<LevelPack>(*audioPlayer, levelPackName);
		spriteLoader = levelPack->createSpriteLoader();
		spriteLoader->preloadTextures();
	} catch (const char* str) {
		// TODO: log error and display to user
		return;
	}

	mainPanel->removeAllTabs();

	attacksListView->setLevelPack(levelPack.get());

	attackPatternsListView->setLevelPack(levelPack.get());

	// TODO: add more left panel panels to here

	if (previewWindow) {
		previewWindow->loadLevelPack(levelPackName);
	} else {
		// TODO: window size from settings
		previewWindow = std::make_shared<LevelPackObjectPreviewWindow>("Preview", 1024, 768, levelPackName);
		std::thread previewWindowThread = std::thread(&LevelPackObjectPreviewWindow::start, &(*previewWindow));
		previewWindowThread.detach();
	}
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

void MainEditorWindow::updateAttack(std::shared_ptr<EditorAttack> attack) {
	levelPack->updateAttack(attack);
	previewWindow->onOriginalLevelPackAttackModified(attack);
}

void MainEditorWindow::updateAttackPattern(std::shared_ptr<EditorAttackPattern> attackPattern) {
	levelPack->updateAttackPattern(attackPattern);
	previewWindow->onOriginalLevelPackAttackPatternModified(attackPattern);
}

void MainEditorWindow::deleteAttack(int id) {
	levelPack->deleteAttack(id);
	previewWindow->deleteAttack(id);

	if (unsavedAttacks.find(id) != unsavedAttacks.end()) {
		unsavedAttacks.erase(id);
	}
}

void MainEditorWindow::deleteAttackPattern(int id) {
	levelPack->deleteAttackPattern(id);
	previewWindow->deleteAttackPattern(id);

	if (unsavedAttackPatterns.find(id) != unsavedAttackPatterns.end()) {
		unsavedAttackPatterns.erase(id);
	}
}

bool MainEditorWindow::handleEvent(sf::Event event) {
	if (EditorWindow::handleEvent(event)) {
		return true;
	}

	if (leftPanel->isFocused()) {
		return leftPanel->handleEvent(event);
	} else if (mainPanel->isFocused()) {
		return mainPanel->handleEvent(event);
	}
	return false;
}

void MainEditorWindow::showClipboardResult(std::string notification) {
	clipboardNotification->setText(notification);
}

void MainEditorWindow::populateLeftPanelLevelPackObjectListPanel(std::shared_ptr<LevelPackObjectsListView> listView,
	std::shared_ptr<LevelPackObjectsListPanel> listPanel, std::function<void()> createLevelPackObject, std::function<void(int)> openLevelPackObjectTab) {
	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

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
		std::lock_guard<std::recursive_mutex> lock(tguiMutex);
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
	if (unsavedAttackPatterns.count(id) > 0) {
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
		std::lock_guard<std::recursive_mutex> lock(tguiMutex);
		std::shared_ptr<AttackPatternEditorPanel> attackPatternEditorPanel = AttackPatternEditorPanel::create(*this, levelPack, *spriteLoader, clipboard, openedObject);
		attackPatternEditorPanel->connect("EnemyPhaseBeginEdit", [this](int enemyPhaseID) {
			openLeftPanelEnemyPhase(enemyPhaseID);
		});
		attackPatternEditorPanel->connect("PlayerBeginEdit", [this]() {
			openLeftPanelPlayer();
		});
		attackPatternEditorPanel->connect("AttackPatternModified", [this](std::shared_ptr<EditorAttackPattern> attackPattern) {
			unsavedAttackPatterns[attackPattern->getID()] = std::dynamic_pointer_cast<LevelPackObject>(attackPattern);
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
		std::lock_guard<std::recursive_mutex> lock(tguiMutex);
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
		std::lock_guard<std::recursive_mutex> lock(tguiMutex);
		std::shared_ptr<AttackPatternEditorPanel> attackPatternEditorPanel = AttackPatternEditorPanel::create(*this, levelPack, *spriteLoader, clipboard, openedObject);
		attackPatternEditorPanel->connect("EnemyPhaseBeginEdit", [this](int enemyPhaseID) {
			openLeftPanelEnemyPhase(enemyPhaseID);
		});
		attackPatternEditorPanel->connect("PlayerBeginEdit", [this]() {
			openLeftPanelPlayer();
		});
		attackPatternEditorPanel->connect("AttackPatternModified", [this](std::shared_ptr<EditorAttackPattern> attackPattern) {
			unsavedAttackPatterns[attackPattern->getID()] = std::dynamic_pointer_cast<LevelPackObject>(attackPattern);
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

void MainEditorWindow::saveAttackChanges(int id) {
	// Do nothing if the attack doesn't have any unsaved changes

	if (unsavedAttacks.count(id) > 0) {
		levelPack->updateAttack(unsavedAttacks[id]);
		unsavedAttacks.erase(id);

		attacksListView->reload();
	}
}

void MainEditorWindow::saveAttackChanges(std::set<size_t> ids) {
	if (ids.size() > 0) {
		for (int id : ids) {
			if (unsavedAttacks.count(id) > 0) {
				levelPack->updateAttack(unsavedAttacks[id]);
				unsavedAttacks.erase(id);
			}
		}
		attacksListView->reload();
	}
}

void MainEditorWindow::saveAttackPatternChanges(int id) {
	// Do nothing if the attack pattern doesn't have any unsaved changes

	if (unsavedAttackPatterns.count(id) > 0) {
		levelPack->updateAttackPattern(unsavedAttackPatterns[id]);
		unsavedAttackPatterns.erase(id);

		attackPatternsListView->reload();
	}
}

void MainEditorWindow::saveAttackPatternChanges(std::set<size_t> ids) {
	if (ids.size() > 0) {
		for (int id : ids) {
			if (unsavedAttackPatterns.count(id) > 0) {
				levelPack->updateAttackPattern(unsavedAttackPatterns[id]);
				unsavedAttackPatterns.erase(id);
			}
		}
		attackPatternsListView->reload();
	}
}

void MainEditorWindow::saveAllChanges() {
	if (unsavedAttacks.size() > 0) {
		for (std::pair<int, std::shared_ptr<LevelPackObject>> changes : unsavedAttacks) {
			levelPack->updateAttack(changes.second);
		}
		unsavedAttacks.clear();
		attacksListView->reload();
	}

	if (unsavedAttackPatterns.size() > 0) {
		for (std::pair<int, std::shared_ptr<LevelPackObject>> changes : unsavedAttackPatterns) {
			levelPack->updateAttackPattern(changes.second);
		}
		unsavedAttackPatterns.clear();
		attackPatternsListView->reload();
	}

	// TODO: add to this
}

std::shared_ptr<LevelPackObjectsListView> MainEditorWindow::getAttacksListView() {
	return attacksListView;
}

std::shared_ptr<LevelPackObjectsListPanel> MainEditorWindow::getAttacksListPanel() {
	return attacksListPanel;
}

std::shared_ptr<LevelPackObjectsListView> MainEditorWindow::getAttackPatternsListView() {
	return attackPatternsListView;
}

std::shared_ptr<LevelPackObjectsListPanel> MainEditorWindow::getAttackPatternsListPanel() {
	return attackPatternsListPanel;
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

bool MainEditorWindow::hasUnsavedChanges() {
	// TODO: add to this
	return unsavedAttacks.size() > 0 || unsavedAttackPatterns.size() > 0
		|| unsavedBulletModels.size() > 0 || unsavedEnemies.size() > 0
		|| unsavedEnemyPhases.size() > 0;
}

void MainEditorWindow::createAttack(bool undoable) {
	int id = levelPack->getNextAttackID();
	if (undoable) {
		attacksListView->getUndoStack().execute(UndoableCommand([this, id]() {
			auto attack = levelPack->createAttack(id);
			previewWindow->onOriginalLevelPackAttackModified(attack);

			// Open the newly created attack
			openLeftPanelAttack(id);

			attacksListView->reload();

			// Select it in attacksListView
			attacksListView->getListView()->setSelectedItem(attacksListView->getIndexFromLevelPackObjectID(id));
		}, [this, id]() {
			levelPack->deleteAttack(id);
			previewWindow->deleteAttack(id);

			attacksListView->reload();
		}));
	} else {
		auto attack = levelPack->createAttack(id);
		previewWindow->onOriginalLevelPackAttackModified(attack);
	}
}

void MainEditorWindow::createAttackPattern(bool undoable) {
	int id = levelPack->getNextAttackPatternID();
	if (undoable) {
		attackPatternsListView->getUndoStack().execute(UndoableCommand([this, id]() {
			auto attackPattern = levelPack->createAttackPattern(id);
			previewWindow->onOriginalLevelPackAttackPatternModified(attackPattern);

			// Open the newly created attack
			openLeftPanelAttackPattern(id);

			attackPatternsListView->reload();

			// Select it in attacksListView
			attackPatternsListView->getListView()->setSelectedItem(attackPatternsListView->getIndexFromLevelPackObjectID(id));
		}, [this, id]() {
			levelPack->deleteAttackPattern(id);
			previewWindow->deleteAttackPattern(id);

			attackPatternsListView->reload();
		}));
	} else {
		auto attackPattern = levelPack->createAttackPattern(id);
		previewWindow->onOriginalLevelPackAttackPatternModified(attackPattern);
	}
}