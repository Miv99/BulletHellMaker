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

const std::string MainEditorWindow::LEFT_PANEL_SPRITE_SHEETS_TAB_NAME = "Sprite Sheets";
const std::string MainEditorWindow::LEFT_PANEL_ATTACK_LIST_TAB_NAME = "Attacks";
// %d = the attack's ID
const std::string MainEditorWindow::LEFT_PANEL_ATTACK_TABS_SET_IDENTIFIER_FORMAT = "Attack %d";
const std::string MainEditorWindow::LEFT_PANEL_ATTACK_PATTERN_LIST_TAB_NAME = "Attack patterns";
// %d = the attack pattern's ID
const std::string MainEditorWindow::LEFT_PANEL_ATTACK_PATTERN_TABS_SET_IDENTIFIER_FORMAT = "Atk. Pattern %d";

// %d = the level pack object's ID
const std::string MainEditorWindow::MAIN_PANEL_ATTACK_TAB_NAME_FORMAT = "Attack %d";
const std::string MainEditorWindow::MAIN_PANEL_ATTACK_PATTERN_TAB_NAME_FORMAT = "Atk. Pattern %d";

// %s = the sprite sheet name
const std::string MainEditorWindow::MAIN_PANEL_SPRITE_SHEET_TAB_NAME_FORMAT = "%s";

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
		// Sprite sheets tree view panel in left panel
		spriteSheetsListPanel = SpriteSheetsListPanel::create(*this);
		spriteSheetsListPanel->getListViewScrollablePanel()->onDoubleClick.connect([this](int index) {
			this->openLeftPanelSpriteSheet(spriteSheetsListPanel->getSpriteSheetNameByIndex(index));
		});
		leftPanel->addTab(LEFT_PANEL_SPRITE_SHEETS_TAB_NAME, spriteSheetsListPanel, true);
	}
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

	searchChildWindow = LevelPackSearchChildWindow::create(*this);
	searchChildWindow->setTitleButtons(tgui::ChildWindow::TitleButton::Close);

	gui->add(menuBar);

	clipboard.getOnCopy()->sink().connect<MainEditorWindow, &MainEditorWindow::showClipboardResult>(this);
	clipboard.getOnPaste()->sink().connect<MainEditorWindow, &MainEditorWindow::showClipboardResult>(this);
}

MainEditorWindow::~MainEditorWindow() {
	clipboard.getOnCopy()->sink().disconnect(this);
	clipboard.getOnPaste()->sink().disconnect(this);
	if (previewWindow) {
		previewWindow->close();
	}
}

void MainEditorWindow::loadLevelPack(std::string levelPackName) {
	searchChildWindow->onMainEditorWindowLevelPackLoad();

	audioPlayer = std::make_shared<AudioPlayer>();
	try {
		levelPack = std::make_shared<LevelPack>(*audioPlayer, levelPackName);
		LevelPack::LoadMetrics levelPackLoadMetrics = levelPack->load();

		spriteLoader = levelPack->getSpriteLoader();
		SpriteLoader::LoadMetrics spriteLoaderLoadMetrics = spriteLoader->loadFromSpriteSheetsFolder();

		// Display load metrics
		std::string combinedMetrics = format("Loaded level pack name \"%s\".\n\n", levelPackName.c_str()) + levelPackLoadMetrics.formatForUser() 
			+ "\n\n" + spriteLoaderLoadMetrics.formatForUser();
		bool levelPackLoadFailed = levelPackLoadMetrics.containsFailedLoads();
		bool spriteLoaderLoadFailed = spriteLoaderLoadMetrics.containsFailedLoads();
		if (levelPackLoadFailed) {
			combinedMetrics += "\n\nWARNING: Some level pack objects failed to load and will be deleted upon the next save.";
		}
		if (spriteLoaderLoadFailed) {
			combinedMetrics += "\n\nWARNING: Some sprite sheets or sprite sheet metafiles failed to load and will neither be editable nor saved.";
		}
		if (!levelPackLoadFailed && !spriteLoaderLoadFailed) {
			combinedMetrics += "\n\nSuccessfully loaded the level pack with no errors.";
		}
		showPopupMessageWindow(combinedMetrics, nullptr);

		loadedEditableLevelPack = true;

	} catch (const std::exception& e) {
		showPopupMessageWindow(format("An exception occurred while loading level pack \"%s\": %s", levelPackName.c_str(), e.what()), nullptr);
		loadedEditableLevelPack = false;
		return;
	}

	mainPanel->removeAllTabs();

	spriteSheetsListPanel->setLevelPack(levelPack.get());
	attacksListView->setLevelPack(levelPack.get());
	attackPatternsListView->setLevelPack(levelPack.get());

	// TODO: add more left panel panels to here

	if (previewWindow) {
		previewWindow->loadLevelPack(levelPackName, spriteLoader);
	}
}

void MainEditorWindow::overwriteAttacks(std::vector<std::shared_ptr<EditorAttack>> attacks, UndoStack* undoStack) {
	std::vector<std::shared_ptr<EditorAttack>> oldAttacks;
	for (std::shared_ptr<EditorAttack> attack : attacks) {
		int id = attack->getID();
		if (unsavedAttacks.find(id) != unsavedAttacks.end()) {
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
				if (unsavedAttacks.find(id) != unsavedAttacks.end()) {
					std::dynamic_pointer_cast<EditorAttack>(unsavedAttacks[id])->load(attack->format());
				} else {
					unsavedAttacks[id] = std::dynamic_pointer_cast<LayerRootLevelPackObject>(std::make_shared<EditorAttack>(attack));
				}

				reloadAttackTab(id);
				attacksListView->reload();
			}
		}, [this, oldAttacks]() {
			for (std::shared_ptr<EditorAttack> attack : oldAttacks) {
				int id = attack->getID();

				// Overwrite existing EditorAttack
				if (unsavedAttacks.find(id) != unsavedAttacks.end()) {
					std::dynamic_pointer_cast<EditorAttack>(unsavedAttacks[id])->load(attack->format());
				} else {
					unsavedAttacks[id] = std::dynamic_pointer_cast<LayerRootLevelPackObject>(std::make_shared<EditorAttack>(attack));
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
			if (unsavedAttacks.find(id) != unsavedAttacks.end()) {
				std::dynamic_pointer_cast<EditorAttack>(unsavedAttacks[id])->load(attack->format());
			} else {
				unsavedAttacks[id] = std::dynamic_pointer_cast<LayerRootLevelPackObject>(std::make_shared<EditorAttack>(attack));
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

void MainEditorWindow::openSearchChildWindow() {
	addChildWindow(searchChildWindow);
}

LevelPackSearchChildWindow::LevelPackSearchFindAllResult MainEditorWindow::findAllInstancesOfSpriteName(std::string spriteSheetName, std::string spriteName) {
	if (!levelPack || !spriteLoader) {
		return LevelPackSearchChildWindow::LevelPackSearchFindAllResult();
	}
	return findAllInstancesOfAnimatable(spriteSheetName, spriteName, true);
}

LevelPackSearchChildWindow::LevelPackSearchFindAllResult MainEditorWindow::findAllInstancesOfAnimationName(std::string spriteSheetName, std::string spriteName) {
	if (!levelPack || !spriteLoader) {
		return LevelPackSearchChildWindow::LevelPackSearchFindAllResult();
	}
	return findAllInstancesOfAnimatable(spriteSheetName, spriteName, false);
}

void MainEditorWindow::updateAttack(std::shared_ptr<EditorAttack> attack) {
	levelPack->updateAttack(attack);
	if (previewWindow) {
		previewWindow->onOriginalLevelPackAttackModified(attack);
	}
}

void MainEditorWindow::updateAttackPattern(std::shared_ptr<EditorAttackPattern> attackPattern) {
	levelPack->updateAttackPattern(attackPattern);
	if (previewWindow) {
		previewWindow->onOriginalLevelPackAttackPatternModified(attackPattern);
	}
}

void MainEditorWindow::deleteAttack(int id) {
	levelPack->deleteAttack(id);
	if (previewWindow) {
		previewWindow->deleteAttack(id);
	}

	if (unsavedAttacks.find(id) != unsavedAttacks.end()) {
		unsavedAttacks.erase(id);
	}
}

void MainEditorWindow::deleteAttackPattern(int id) {
	levelPack->deleteAttackPattern(id);
	if (previewWindow) {
		previewWindow->deleteAttackPattern(id);
	}

	if (unsavedAttackPatterns.find(id) != unsavedAttackPatterns.end()) {
		unsavedAttackPatterns.erase(id);
	}
}


void MainEditorWindow::reloadSpriteLoader() {
	if (!spriteLoader) {
		return;
	}

	std::set<std::string> oldLoadedSpriteSheets = spriteLoader->getLoadedSpriteSheetNamesAsSet();

	SpriteLoader::LoadMetrics loadMetrics = spriteSheetsListPanel->reloadSpriteLoaderAndList();
	// The level pack being edited by this MainEditorWindow is the same SpriteLoader as the one
	// being used in previewWindow's level pack, so don't reload the one in previewWindow.

	std::set<std::string> newLoadedSpriteSheets = spriteLoader->getLoadedSpriteSheetNamesAsSet();
	for (std::string spriteSheetName : oldLoadedSpriteSheets) {
		std::string tabName = format(MAIN_PANEL_SPRITE_SHEET_TAB_NAME_FORMAT, spriteSheetName.c_str());
		if (mainPanel->hasTab(tabName)) {
			if (newLoadedSpriteSheets.find(spriteSheetName) == newLoadedSpriteSheets.end()
				|| (newLoadedSpriteSheets.find(spriteSheetName) != newLoadedSpriteSheets.end()
					&& (spriteLoader->spriteSheetFailedImageLoad(spriteSheetName) || spriteLoader->spriteSheetFailedMetafileLoad(spriteSheetName)))) {
				// Close all sprite sheet tabs that failed to load or no longer exist
				mainPanel->removeTab(tabName);
			} else {
				// Reload image of existing sprite sheet tabs
				std::dynamic_pointer_cast<SpriteSheetMetafileEditor>(mainPanel->getTab(tabName))->loadImage(levelPack->getName(), spriteLoader, spriteSheetName);
			}
		}
	}

	// Display load metrics
	showPopupMessageWindow(loadMetrics.formatForUser(), nullptr);
	
	// Reset preview because existing sprites will continue to use the old textures
	if (previewWindow) {
		previewWindow->resetPreview();
	}
}

bool MainEditorWindow::handleEvent(sf::Event event) {
	if (EditorWindow::handleEvent(event)) {
		return true;
	}

	if ((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) 
		&& event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F) {

		openSearchChildWindow();
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
	addButton->setToolTip(createToolTip("Creates a new object"));
	addButton->onPress.connect([this, createLevelPackObject]() {
		createLevelPackObject();
	});
	listPanel->add(addButton);

	// Save all button
	auto saveAllButton = tgui::Button::create();
	saveAllButton->setText("S");
	saveAllButton->setPosition(tgui::bindRight(addButton), 0);
	saveAllButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	saveAllButton->setToolTip(createToolTip("Saves all objects in this list"));
	saveAllButton->onPress.connect([this, listView]() {
		listView->manualSaveAll();
	});
	listPanel->add(saveAllButton);

	// Sort button
	auto sortButton = tgui::Button::create();
	sortButton->setText("=");
	sortButton->setPosition(tgui::bindRight(saveAllButton), 0);
	sortButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	sortButton->setToolTip(createToolTip("Toggles between sorting this list by either ID number or name"));
	sortButton->onPress.connect([this, listView]() {
		listView->cycleSortOption();
	});
	listPanel->add(sortButton);

	// List view
	listView->setPosition(0, tgui::bindBottom(addButton));
	listView->setSize("100%", tgui::bindHeight(listPanel) - tgui::bindBottom(addButton) - DEFAULT_SCROLLBAR_SIZE);
	{
		// Right click menu
		// Menu for single selection
		auto rightClickMenuPopupSingleSelection = createMenuPopup({
			std::make_pair("Open", [this, listView, openLevelPackObjectTab]() {
				openLevelPackObjectTab(listView->getLevelPackObjectIDFromIndex(listView->getSelectedItemIndex()));
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
		listView->onRightClick.connect([this, listView, rightClickMenuPopupSingleSelection, rightClickMenuPopupMultiSelection](int index) {
			if (index == -1) {
				return;
			}

			std::set<std::size_t> selectedItemIndices = listView->getSelectedItemIndices();
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
				listView->setSelectedItem(index);

				// Open the menu normally
				addPopupWidget(rightClickMenuPopupSingleSelection, mousePos.x, mousePos.y, 150, rightClickMenuPopupSingleSelection->getSize().y);
			}
		});
	}
	listView->onDoubleClick.connect([this, listView, openLevelPackObjectTab](int index) {
		openLevelPackObjectTab(listView->getLevelPackObjectIDFromIndex(index));
	});
	listPanel->add(listView);
}

LevelPackSearchChildWindow::LevelPackSearchFindAllResult MainEditorWindow::findAllInstancesOfAnimatable(std::string spriteSheetName, std::string animatableName, bool mustBeSprite) {
	LevelPackSearchChildWindow::LevelPackSearchFindAllResult result;

	// Search the sprite sheet for the sprite name
	if (levelPack->getSpriteLoader()->hasSpriteSheet(spriteSheetName)) {
		std::shared_ptr<SpriteSheet> spriteSheet;
		if (unsavedSpriteSheets.find(spriteSheetName) != unsavedSpriteSheets.end()) {
			spriteSheet = unsavedSpriteSheets[spriteSheetName];
		} else {
			spriteSheet = levelPack->getSpriteLoader()->getSpriteSheet(spriteSheetName);
		}

		if (spriteSheet->hasSpriteData(animatableName)) {
			std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> node = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
			node->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::SPRITE_SHEET;
			node->name = spriteSheetName;
			result.resultNodes.push_back(node);
		}
	}

	// Search the player
	{
		std::shared_ptr<EditorPlayer> player;
		if (unsavedPlayer) {
			player = unsavedPlayer;
		} else {
			player = levelPack->getPlayer();
		}

		std::vector<std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode>> children;

		std::vector<std::shared_ptr<PlayerPowerTier>> powerTiers = player->getPowerTiers();
		for (int i = 0; i < powerTiers.size(); i++) {
			const EntityAnimatableSet& animatableSet = powerTiers[i]->getAnimatableSet();

			std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> powerTierNode 
				= findAllInstancesOfAnimatableInAnimatableSet(animatableSet, spriteSheetName, animatableName, mustBeSprite);

			if (powerTierNode) {
				powerTierNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::PLAYER_POWER_TIER;
				powerTierNode->id = i;

				children.push_back(powerTierNode);
			}
		}

		if (!children.empty()) {
			std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> node = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
			node->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::PLAYER;
			node->children = children;
			result.resultNodes.push_back(node);
		}
	}

	// Search all enemies
	for (auto it = levelPack->getEnemyIteratorBegin(); it != levelPack->getEnemyIteratorEnd(); it++) {
		std::shared_ptr<EditorEnemy> enemy;
		if (unsavedEnemies.find(it->second->getID()) != unsavedEnemies.end()) {
			enemy = std::dynamic_pointer_cast<EditorEnemy>(unsavedEnemies[it->second->getID()]);
		} else {
			enemy = it->second;
		}

		std::vector<std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode>> children;

		std::vector<EntityAnimatableSet> sets = enemy->getAnimatableSets();
		for (int i = 0; i < sets.size(); i++) {
			std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> phaseUsageNode
				= findAllInstancesOfAnimatableInAnimatableSet(sets[i], spriteSheetName, animatableName, mustBeSprite);

			if (phaseUsageNode) {
				phaseUsageNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::ENEMY_PHASE_USAGE;
				phaseUsageNode->id = i;

				children.push_back(phaseUsageNode);
			}
		}

		if (!children.empty()) {
			std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> node = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
			node->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::ENEMY;
			node->id = it->first;
			node->name = enemy->getName();
			node->children = children;
			result.resultNodes.push_back(node);
		}
	}

	// Search all bullet models
	for (auto it = levelPack->getBulletModelIteratorBegin(); it != levelPack->getBulletModelIteratorEnd(); it++) {
		std::shared_ptr<BulletModel> bulletModel;
		if (unsavedBulletModels.find(it->second->getID()) != unsavedBulletModels.end()) {
			bulletModel = std::dynamic_pointer_cast<BulletModel>(unsavedBulletModels[it->second->getID()]);
		} else {
			bulletModel = it->second;
		}

		std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> bulletModelNode;

		// Check bullet model main animatable
		Animatable animatable = bulletModel->getAnimatable();
		if (animatable.getSpriteSheetName() == spriteSheetName
			&& ((animatable.isSprite() && mustBeSprite) || (!animatable.isSprite() && !mustBeSprite))
			&& animatable.getAnimatableName() == animatableName) {

			bulletModelNode = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
			bulletModelNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::BULLET_MODEL;
			bulletModelNode->id = bulletModel->getID();
			bulletModelNode->name = bulletModel->getName();

			std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> descriptorNode = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
			descriptorNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::DESCRIPTOR;
			descriptorNode->name = "Primary sprite/animation";
			bulletModelNode->children.push_back(descriptorNode);
		}

		// Check bullet model base sprite
		if (mustBeSprite) {
			animatable = bulletModel->getBaseSprite();
			if (animatable.getSpriteSheetName() == spriteSheetName && animatable.isSprite() && animatable.getAnimatableName() == animatableName) {
				if (!bulletModelNode) {
					bulletModelNode = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
					bulletModelNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::BULLET_MODEL;
					bulletModelNode->id = bulletModel->getID();
				}

				std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> descriptorNode = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
				descriptorNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::DESCRIPTOR;
				descriptorNode->name = "Base sprite";
				bulletModelNode->children.push_back(descriptorNode);
			}
		}

		if (bulletModelNode) {
			result.resultNodes.push_back(bulletModelNode);
		}
	}

	// Search all attacks' EMPs
	for (auto it = levelPack->getAttackIteratorBegin(); it != levelPack->getAttackIteratorEnd(); it++) {
		std::shared_ptr<EditorAttack> attack;
		if (unsavedAttacks.find(it->second->getID()) != unsavedAttacks.end()) {
			attack = std::dynamic_pointer_cast<EditorAttack>(unsavedAttacks[it->second->getID()]);
		} else {
			attack = it->second;
		}

		std::vector<std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode>> children;

		std::stack<std::shared_ptr<EditorMovablePoint>> empStack;
		empStack.push(attack->getMainEMP());
		while (!empStack.empty()) {
			std::shared_ptr<EditorMovablePoint> emp = empStack.top();
			empStack.pop();

			std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> empNode;

			// Check EMP main animatable
			Animatable animatable = emp->getAnimatable();
			if (animatable.getSpriteSheetName() == spriteSheetName 
				&& ((animatable.isSprite() && mustBeSprite) || (!animatable.isSprite() && !mustBeSprite))
				&& animatable.getAnimatableName() == animatableName) {

				empNode = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
				empNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::EMP;
				empNode->id = emp->getID();

				std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> descriptorNode = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
				descriptorNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::DESCRIPTOR;
				descriptorNode->name = "Primary sprite/animation";
				empNode->children.push_back(descriptorNode);
			}

			// Check EMP base sprite
			if (mustBeSprite) {
				animatable = emp->getBaseSprite();
				if (animatable.getSpriteSheetName() == spriteSheetName && animatable.isSprite() && animatable.getAnimatableName() == animatableName) {
					if (!empNode) {
						empNode = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
						empNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::EMP;
						empNode->id = emp->getID();
					}

					std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> descriptorNode = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
					descriptorNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::DESCRIPTOR;
					descriptorNode->name = "Base sprite";
					empNode->children.push_back(descriptorNode);
				}
			}

			if (empNode) {
				children.push_back(empNode);
			}

			for (std::shared_ptr<EditorMovablePoint> empChild : emp->getChildren()) {
				empStack.push(empChild);
			}
		}

		if (!children.empty()) {
			std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> node = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
			node->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::ATTACK;
			node->name = attack->getName();
			node->id = it->first;
			node->children = children;
			result.resultNodes.push_back(node);
		}
	}

	return result;
}

std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> MainEditorWindow::findAllInstancesOfAnimatableInAnimatableSet(const EntityAnimatableSet& animatableSet, 
	const std::string& spriteSheetName, const std::string& animatableName, bool mustBeSprite) {

	std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> node;
	
	Animatable animatable = animatableSet.getAttackAnimatable();
	if (animatable.getSpriteSheetName() == spriteSheetName
		&& ((animatable.isSprite() && mustBeSprite) || (!animatable.isSprite() && !mustBeSprite))
		&& animatable.getAnimatableName() == animatableName) {

		node = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();

		std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> descriptorNode = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
		descriptorNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::DESCRIPTOR;
		descriptorNode->name = "Attack sprite/animation";
		node->children.push_back(descriptorNode);
	}

	animatable = animatableSet.getIdleAnimatable();
	if (animatable.getSpriteSheetName() == spriteSheetName
		&& ((animatable.isSprite() && mustBeSprite) || (!animatable.isSprite() && !mustBeSprite))
		&& animatable.getAnimatableName() == animatableName) {

		if (!node) {
			node = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
		}

		std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> descriptorNode = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
		descriptorNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::DESCRIPTOR;
		descriptorNode->name = "Idle sprite/animation";
		node->children.push_back(descriptorNode);
	}

	animatable = animatableSet.getMovementAnimatable();
	if (animatable.getSpriteSheetName() == spriteSheetName
		&& ((animatable.isSprite() && mustBeSprite) || (!animatable.isSprite() && !mustBeSprite))
		&& animatable.getAnimatableName() == animatableName) {

		if (!node) {
			node = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
		}

		std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> descriptorNode = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
		descriptorNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::DESCRIPTOR;
		descriptorNode->name = "Movement sprite/animation";
		node->children.push_back(descriptorNode);
	}

	if (std::dynamic_pointer_cast<ParticleExplosionDeathAction>(animatableSet.getDeathAction())) {
		animatable = std::dynamic_pointer_cast<ParticleExplosionDeathAction>(animatableSet.getDeathAction())->getAnimatable();
		if (animatable.getSpriteSheetName() == spriteSheetName
			&& ((animatable.isSprite() && mustBeSprite) || (!animatable.isSprite() && !mustBeSprite))
			&& animatable.getAnimatableName() == animatableName) {

			if (!node) {
				node = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
			}

			std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> descriptorNode = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
			descriptorNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::DESCRIPTOR;
			descriptorNode->name = "Death action: particle explosion";
			node->children.push_back(descriptorNode);
		}
	} else if (std::dynamic_pointer_cast<PlayAnimatableDeathAction>(animatableSet.getDeathAction())) {
		animatable = std::dynamic_pointer_cast<PlayAnimatableDeathAction>(animatableSet.getDeathAction())->getAnimatable();
		if (animatable.getSpriteSheetName() == spriteSheetName
			&& ((animatable.isSprite() && mustBeSprite) || (!animatable.isSprite() && !mustBeSprite))
			&& animatable.getAnimatableName() == animatableName) {

			if (!node) {
				node = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
			}

			std::shared_ptr<LevelPackSearchChildWindow::LevelPackSearchResultNode> descriptorNode = std::make_shared<LevelPackSearchChildWindow::LevelPackSearchResultNode>();
			descriptorNode->type = LevelPackSearchChildWindow::LEVEL_PACK_SEARCH_RESULT_OBJECT_TYPE::DESCRIPTOR;
			descriptorNode->name = "Death action: show sprite/animation";
			node->children.push_back(descriptorNode);
		}
	}

	return node;
}

void MainEditorWindow::openLeftPanelSpriteSheet(std::string spriteSheetName) {
	// Open sprite sheets tab in left panel if not already open
	if (leftPanel->getSelectedTab() != LEFT_PANEL_SPRITE_SHEETS_TAB_NAME) {
		leftPanel->selectTab(LEFT_PANEL_SPRITE_SHEETS_TAB_NAME);
	}
	// Select the sprite sheet in spriteSheetsListPanel's list view
	spriteSheetsListPanel->selectSpriteSheetByName(spriteSheetName);

	if (spriteLoader->spriteSheetFailedImageLoad(spriteSheetName)) {
		// Focus leftPanel afterwards because user interaction with it is the only caller of this function
		showPopupMessageWindow(format("Could not open sprite sheet because its image file failed to load. \
Go to \"File > Reload sprites/animations\" to reload sprite sheets.", spriteLoader->formatPathToSpriteSheetImage(spriteSheetName)), leftPanel.get());
		return;
	}

	if (spriteLoader->spriteSheetFailedMetafileLoad(spriteSheetName)) {
		// Focus leftPanel afterwards because user interaction with it is the only caller of this function
		showPopupMessageWindow(format("Could not open sprite sheet because its metafile failed to load. \
The metafile (\"%s\") might be corrupted due to manual editing outside of BulletHellMaker and would require manual deletion or editing to fix it. \
Go to \"File > Reload sprites/animations\" to reload sprite sheets afterwards.", spriteLoader->formatPathToSpriteSheetMetafile(spriteSheetName).c_str()), leftPanel.get());
		return;
	}

	// Get the sprite sheet
	std::shared_ptr<SpriteSheet> openedSpriteSheet;
	if (unsavedSpriteSheets.find(spriteSheetName) != unsavedSpriteSheets.end()) {
		// There are unsaved changes for this sprite sheet, so open the one with unsaved changes
		openedSpriteSheet = unsavedSpriteSheets.at(spriteSheetName);
	} else {
		// Make a copy of the sprite sheet in the LevelPack so that changes can be applied/discarded
		// whenever the user wants instead of modifying the LevelPack directly.
		openedSpriteSheet = std::make_shared<SpriteSheet>(levelPack->getSpriteLoader()->getSpriteSheet(spriteSheetName));
	}

	// Open the tab in mainPanel
	std::string tabName = format(MAIN_PANEL_SPRITE_SHEET_TAB_NAME_FORMAT, spriteSheetName.c_str());
	if (mainPanel->hasTab(tabName)) {
		// Tab already exists, so just select it
		mainPanel->selectTab(tabName);
	} else {
		// Create the tab
		std::lock_guard<std::recursive_mutex> lock(tguiMutex);
		std::shared_ptr<SpriteSheetMetafileEditor> spriteSheetsMetafileSpritesEditor = SpriteSheetMetafileEditor::create(*this, clipboard);
		spriteSheetsMetafileSpritesEditor->loadSpriteSheet(levelPack->getName(), spriteLoader, openedSpriteSheet);
		spriteSheetsMetafileSpritesEditor->onMetafileModify.connect([this](std::shared_ptr<SpriteSheet> spriteSheet) {
			unsavedSpriteSheets[spriteSheet->getName()] = spriteSheet;
			spriteSheetsListPanel->reloadListOnly();

			if (previewWindow) {
				previewWindow->onOriginalLevelPackSpriteSheetModified(spriteSheet);
			}
		});
		mainPanel->addTab(tabName, spriteSheetsMetafileSpritesEditor, true, true);
	}
}

void MainEditorWindow::openLeftPanelAttack(int attackID) {
	// Open attacks tab in left panel if not already open
	if (leftPanel->getSelectedTab() != LEFT_PANEL_ATTACK_LIST_TAB_NAME) {
		leftPanel->selectTab(LEFT_PANEL_ATTACK_LIST_TAB_NAME);
	}
	// Select the attack in attacksListView
	attacksListView->setSelectedItem(attacksListView->getIndexFromLevelPackObjectID(attackID));

	// Get the attack
	std::shared_ptr<EditorAttack> openedAttack;
	if (unsavedAttacks.find(attackID) != unsavedAttacks.end()) {
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
		attackEditorPanel->onAttackPatternBeginEdit.connect([this](int attackPatternID) {
			openLeftPanelAttackPattern(attackPatternID);
		});
		attackEditorPanel->onAttackModify.connect([this](std::shared_ptr<EditorAttack> attack) {
			unsavedAttacks[attack->getID()] = std::dynamic_pointer_cast<LayerRootLevelPackObject>(attack);
			attacksListView->reload();

			if (previewWindow) {
				previewWindow->onOriginalLevelPackAttackModified(attack);
			}
		});
		std::string tabName = format(MAIN_PANEL_ATTACK_TAB_NAME_FORMAT, attackID);
		mainPanel->addTab(tabName, attackEditorPanel, true, true);
		mainPanel->setTabOnSelectFunction(tabName, [this, openedAttack]() {
			if (previewWindow) {
				previewWindow->previewAttack(openedAttack);
			}
		});
	}

	// View preview
	if (previewWindow) {
		previewWindow->previewAttack(openedAttack);
	}
}

void MainEditorWindow::openLeftPanelAttackPattern(int id) {
	// Open tab in left panel if not already open
	if (leftPanel->getSelectedTab() != LEFT_PANEL_ATTACK_PATTERN_LIST_TAB_NAME) {
		leftPanel->selectTab(LEFT_PANEL_ATTACK_PATTERN_LIST_TAB_NAME);
	}
	// Select the object in the list view
	attackPatternsListView->setSelectedItem(attackPatternsListView->getIndexFromLevelPackObjectID(id));

	// Get the object
	std::shared_ptr<EditorAttackPattern> openedObject;
	if (unsavedAttackPatterns.find(id) != unsavedAttackPatterns.end()) {
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
		attackPatternEditorPanel->onEnemyPhaseBeginEdit.connect([this](int enemyPhaseID) {
			openLeftPanelEnemyPhase(enemyPhaseID);
		});
		attackPatternEditorPanel->onPlayerBeginEdit.connect([this]() {
			openLeftPanelPlayer();
		});
		attackPatternEditorPanel->onAttackPatternModify.connect([this](std::shared_ptr<EditorAttackPattern> attackPattern) {
			unsavedAttackPatterns[attackPattern->getID()] = std::dynamic_pointer_cast<LayerRootLevelPackObject>(attackPattern);
			attackPatternsListView->reload();

			if (previewWindow) {
				previewWindow->onOriginalLevelPackAttackPatternModified(attackPattern);
			}
		});
		std::string tabName = format(MAIN_PANEL_ATTACK_PATTERN_TAB_NAME_FORMAT, id);
		mainPanel->addTab(tabName, attackPatternEditorPanel, true, true);
		mainPanel->setTabOnSelectFunction(tabName, [this, openedObject]() {
			if (previewWindow) {
				previewWindow->previewAttackPattern(openedObject);
			}
		});
	}

	// View preview
	if (previewWindow) {
		previewWindow->previewAttackPattern(openedObject);
	}
}

void MainEditorWindow::overwriteAttackPatterns(std::vector<std::shared_ptr<EditorAttackPattern>> objects, UndoStack* undoStack) {
	std::vector<std::shared_ptr<EditorAttackPattern>> oldObjects;
	for (std::shared_ptr<EditorAttackPattern> obj : objects) {
		int id = obj->getID();
		if (unsavedAttackPatterns.find(id) != unsavedAttackPatterns.end()) {
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
				if (unsavedAttackPatterns.find(id) != unsavedAttackPatterns.end()) {
					std::dynamic_pointer_cast<EditorAttackPattern>(unsavedAttackPatterns[id])->load(obj->format());
				} else {
					unsavedAttackPatterns[id] = std::dynamic_pointer_cast<LayerRootLevelPackObject>(std::make_shared<EditorAttackPattern>(obj));
				}

				reloadAttackPatternTab(id);
				attackPatternsListView->reload();
			}
		}, [this, oldObjects]() {
			for (std::shared_ptr<EditorAttackPattern> obj : oldObjects) {
				int id = obj->getID();

				// Overwrite existing object
				if (unsavedAttackPatterns.find(id) != unsavedAttackPatterns.end()) {
					std::dynamic_pointer_cast<EditorAttackPattern>(unsavedAttackPatterns[id])->load(obj->format());
				} else {
					unsavedAttackPatterns[id] = std::dynamic_pointer_cast<LayerRootLevelPackObject>(std::make_shared<EditorAttackPattern>(obj));
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
			if (unsavedAttackPatterns.find(id) != unsavedAttackPatterns.end()) {
				std::dynamic_pointer_cast<EditorAttackPattern>(unsavedAttackPatterns[id])->load(obj->format());
			} else {
				unsavedAttackPatterns[id] = std::dynamic_pointer_cast<LayerRootLevelPackObject>(std::make_shared<EditorAttackPattern>(obj));
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
		if (unsavedAttacks.find(attackID) != unsavedAttacks.end()) {
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
		attackEditorPanel->onAttackPatternBeginEdit.connect([this](int attackPatternID) {
			openLeftPanelAttackPattern(attackPatternID);
		});
		attackEditorPanel->onAttackModify.connect([this](std::shared_ptr<EditorAttack> attack) {
			unsavedAttacks[attack->getID()] = std::dynamic_pointer_cast<LayerRootLevelPackObject>(attack);
			attacksListView->reload();
		});
		std::string tabName = format(MAIN_PANEL_ATTACK_TAB_NAME_FORMAT, attackID);
		mainPanel->insertTab(tabName, attackEditorPanel, tabIndex, tabWasSelected, true);
		mainPanel->setTabOnSelectFunction(tabName, [this, openedAttack]() {
			if (previewWindow) {
				previewWindow->previewAttack(openedAttack);
			}
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
		if (unsavedAttackPatterns.find(id) != unsavedAttackPatterns.end()) {
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
		attackPatternEditorPanel->onEnemyPhaseBeginEdit.connect([this](int enemyPhaseID) {
			openLeftPanelEnemyPhase(enemyPhaseID);
		});
		attackPatternEditorPanel->onPlayerBeginEdit.connect([this]() {
			openLeftPanelPlayer();
		});
		attackPatternEditorPanel->onAttackPatternModify.connect([this](std::shared_ptr<EditorAttackPattern> attackPattern) {
			unsavedAttackPatterns[attackPattern->getID()] = std::dynamic_pointer_cast<LayerRootLevelPackObject>(attackPattern);
			attackPatternsListView->reload();

			if (previewWindow) {
				previewWindow->onOriginalLevelPackAttackPatternModified(attackPattern);
			}
		});
		std::string tabName = format(MAIN_PANEL_ATTACK_PATTERN_TAB_NAME_FORMAT, id);
		mainPanel->insertTab(tabName, attackPatternEditorPanel, tabIndex, tabWasSelected, true);
		mainPanel->setTabOnSelectFunction(tabName, [this, openedObject]() {
			if (previewWindow) {
				previewWindow->previewAttackPattern(openedObject);
			}
		});
	}
}

void MainEditorWindow::saveAttackChanges(int id) {
	// Do nothing if the attack doesn't have any unsaved changes

	if (unsavedAttacks.find(id) != unsavedAttacks.end()) {
		levelPack->updateAttack(unsavedAttacks[id]);
		unsavedAttacks.erase(id);

		attacksListView->reload();

		// TODO: levelPack->saveAttacks() to write to file
	}
}

void MainEditorWindow::saveAttackChanges(std::set<size_t> ids) {
	if (ids.size() > 0) {
		for (int id : ids) {
			if (unsavedAttacks.find(id) != unsavedAttacks.end()) {
				levelPack->updateAttack(unsavedAttacks[id]);
				unsavedAttacks.erase(id);
			}
		}
		attacksListView->reload();

		// TODO: levelPack->saveAttacks() to write to file
	}
}

void MainEditorWindow::saveAttackPatternChanges(int id) {
	// Do nothing if the attack pattern doesn't have any unsaved changes

	if (unsavedAttackPatterns.find(id) != unsavedAttackPatterns.end()) {
		levelPack->updateAttackPattern(unsavedAttackPatterns[id]);
		unsavedAttackPatterns.erase(id);

		attackPatternsListView->reload();

		// TODO: levelPack->saveAttackPatterns() to write to file
	}
}

void MainEditorWindow::saveAttackPatternChanges(std::set<size_t> ids) {
	if (ids.size() > 0) {
		for (int id : ids) {
			if (unsavedAttackPatterns.find(id) != unsavedAttackPatterns.end()) {
				levelPack->updateAttackPattern(unsavedAttackPatterns[id]);
				unsavedAttackPatterns.erase(id);
			}
		}
		attackPatternsListView->reload();

		// TODO: levelPack->saveAttackPatterns() to write to file
	}
}

void MainEditorWindow::saveSpriteSheetChanges(std::string spriteSheetName) {
	// Do nothing if the sprite sheet doesn't have any unsaved changes

	if (unsavedSpriteSheets.find(spriteSheetName) != unsavedSpriteSheets.end()) {
		levelPack->updateSpriteSheet(unsavedSpriteSheets.at(spriteSheetName));
		unsavedSpriteSheets.erase(spriteSheetName);

		spriteSheetsListPanel->reloadListOnly();

		// TODO: levelPack->saveSpriteSheets() to write to file
	}
}

void MainEditorWindow::saveSpriteSheetChanges(std::set<std::string> spriteSheetNames) {
	// Do nothing if the sprite sheet doesn't have any unsaved changes

	if (spriteSheetNames.size() > 0) {
		for (std::string spriteSheetName : spriteSheetNames) {
			if (unsavedSpriteSheets.find(spriteSheetName) != unsavedSpriteSheets.end()) {
				levelPack->getSpriteLoader()->updateSpriteSheet(unsavedSpriteSheets.at(spriteSheetName));
				unsavedSpriteSheets.erase(spriteSheetName);
			}
		}

		spriteSheetsListPanel->reloadListOnly();
		// TODO: levelPack->saveSpriteSheets() to write to file
	}
}

void MainEditorWindow::saveAllChanges() {
	if (unsavedAttacks.size() > 0) {
		for (std::pair<int, std::shared_ptr<LayerRootLevelPackObject>> changes : unsavedAttacks) {
			levelPack->updateAttack(changes.second);
		}
		unsavedAttacks.clear();
		attacksListView->reload();

		// TODO: levelPack->saveAttacks() to write to file
	}

	if (unsavedAttackPatterns.size() > 0) {
		for (std::pair<int, std::shared_ptr<LayerRootLevelPackObject>> changes : unsavedAttackPatterns) {
			levelPack->updateAttackPattern(changes.second);
		}
		unsavedAttackPatterns.clear();
		attackPatternsListView->reload();

		// TODO: levelPack->saveAttackPatterns() to write to file
	}

	if (unsavedSpriteSheets.size() > 0) {
		for (std::pair<std::string, std::shared_ptr<SpriteSheet>> changes : unsavedSpriteSheets) {
			levelPack->getSpriteLoader()->updateSpriteSheet(changes.second);
		}
		unsavedSpriteSheets.clear();
		spriteSheetsListPanel->reloadListOnly();
		// TODO: levelPack->saveSpriteSheets() to write to file
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

std::map<int, std::shared_ptr<LayerRootLevelPackObject>>& MainEditorWindow::getUnsavedAttacks() {
	return unsavedAttacks;
}

std::map<int, std::shared_ptr<LayerRootLevelPackObject>>& MainEditorWindow::getUnsavedAttackPatterns() {
	return unsavedAttackPatterns;
}

std::map<int, std::shared_ptr<LayerRootLevelPackObject>>& MainEditorWindow::getUnsavedEnemies() {
	return unsavedEnemies;
}

std::map<int, std::shared_ptr<LayerRootLevelPackObject>>& MainEditorWindow::getUnsavedEnemyPhases() {
	return unsavedEnemyPhases;
}

std::map<int, std::shared_ptr<LayerRootLevelPackObject>>& MainEditorWindow::getUnsavedBulletModels() {
	return unsavedBulletModels;
}

std::map<std::string, std::shared_ptr<SpriteSheet>>& MainEditorWindow::getUnsavedSpriteSheets() {
	return unsavedSpriteSheets;
}

bool MainEditorWindow::hasUnsavedChanges() {
	// TODO: add to this
	return unsavedAttacks.size() > 0 || unsavedAttackPatterns.size() > 0
		|| unsavedBulletModels.size() > 0 || unsavedEnemies.size() > 0
		|| unsavedEnemyPhases.size() > 0;
}

void MainEditorWindow::openPreviewWindow() {
	if (loadedEditableLevelPack) {
		if (!previewWindow) {
			// TODO: window size from settings
			previewWindow = std::make_shared<LevelPackObjectPreviewWindow>("Preview", 1024, 768, levelPack->getName(), spriteLoader);
		}

		previewWindow->reopenWindow();
	}
}

void MainEditorWindow::createAttack(bool undoable) {
	int id = levelPack->getNextAttackID();
	if (undoable) {
		attacksListView->getUndoStack().execute(UndoableCommand([this, id]() {
			auto attack = levelPack->createAttack(id);
			if (previewWindow) {
				previewWindow->onOriginalLevelPackAttackModified(attack);
			}

			// Open the newly created attack
			openLeftPanelAttack(id);

			attacksListView->reload();

			// Select it in attacksListView
			attacksListView->setSelectedItem(attacksListView->getIndexFromLevelPackObjectID(id));
		}, [this, id]() {
			levelPack->deleteAttack(id);
			if (previewWindow) {
				previewWindow->deleteAttack(id);
			}

			attacksListView->reload();
		}));
	} else {
		auto attack = levelPack->createAttack(id);
		if (previewWindow) {
			previewWindow->onOriginalLevelPackAttackModified(attack);
		}
	}
}

void MainEditorWindow::createAttackPattern(bool undoable) {
	int id = levelPack->getNextAttackPatternID();
	if (undoable) {
		attackPatternsListView->getUndoStack().execute(UndoableCommand([this, id]() {
			auto attackPattern = levelPack->createAttackPattern(id);
			if (previewWindow) {
				previewWindow->onOriginalLevelPackAttackPatternModified(attackPattern);
			}

			// Open the newly created attack
			openLeftPanelAttackPattern(id);

			attackPatternsListView->reload();

			// Select it in attacksListView
			attackPatternsListView->setSelectedItem(attackPatternsListView->getIndexFromLevelPackObjectID(id));
		}, [this, id]() {
			levelPack->deleteAttackPattern(id);
			if (previewWindow) {
				previewWindow->deleteAttackPattern(id);
			}

			attackPatternsListView->reload();
		}));
	} else {
		auto attackPattern = levelPack->createAttackPattern(id);
		if (previewWindow) {
			previewWindow->onOriginalLevelPackAttackPatternModified(attackPattern);
		}
	}
}