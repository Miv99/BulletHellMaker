#include <Editor/AttackPattern/AttackPatternEditorPanel.h>

const std::string AttackPatternEditorPanel::PROPERTIES_TAB_NAME = "Properties";
const std::string AttackPatternEditorPanel::MOVEMENT_TAB_NAME = "Movement";
const int AttackPatternEditorPanel::USED_BY_ID_MAP_PLAYER_RESERVED_ID = -1;

AttackPatternEditorPanel::AttackPatternEditorPanel(MainEditorWindow& mainEditorWindow, std::shared_ptr<LevelPack> levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorAttackPattern> attackPattern, int undoStackSize) 
	: mainEditorWindow(mainEditorWindow), levelPack(levelPack), spriteLoader(spriteLoader), clipboard(clipboard), undoStack(UndoStack(undoStackSize)), attackPattern(attackPattern) {
	// Tabs
	tabs = TabsWithPanel::create(mainEditorWindow);
	tabs->setPosition(0, 0);
	tabs->setSize("100%", "100%");
	add(tabs);

	// Properties tab
	propertiesPanel = AttackPatternEditorPropertiesPanel::create(mainEditorWindow, clipboard, attackPattern);
	propertiesPanel->connect("AttackPatternModified", [this]() {
		onAttackPatternModify.emit(this, this->attackPattern);
	});
	propertiesPanel->setSize("100%", "100%");
	tabs->addTab(PROPERTIES_TAB_NAME, propertiesPanel, true, false);

	// Populate the usedBy list when the level pack is changed
	levelPack->getOnChange()->sink().connect<AttackPatternEditorPanel, &AttackPatternEditorPanel::onLevelPackChange>(this);
	// Initial population
	populatePropertiesUsedByList();

	propertiesPanel->getUsedByPanel()->getListView()->connect("DoubleClicked", [&](int index) {
		if (usedByIDMap.count(index) > 0) {
			if (usedByIDMap[index] == USED_BY_ID_MAP_PLAYER_RESERVED_ID) {
				onPlayerBeginEdit.emit(this);
			} else {
				onEnemyPhaseBeginEdit.emit(this, usedByIDMap[index]);
			}
		}
	});

	// Right click menu for usedBy
	auto rightClickMenuPopup = createMenuPopup({
		std::make_pair("Edit", [this]() {
			if (usedByRightClickedID == USED_BY_ID_MAP_PLAYER_RESERVED_ID) {
				onPlayerBeginEdit.emit(this);
			} else {
				onEnemyPhaseBeginEdit.emit(this, usedByRightClickedID);
			}
		})
	});
	propertiesPanel->getUsedByPanel()->getListView()->connect("RightClicked", [this, rightClickMenuPopup](int index) {
		if (this->usedByIDMap.count(index) > 0) {
			this->usedByRightClickedID = usedByIDMap[index];
			// Open right click menu
			this->mainEditorWindow.addPopupWidget(rightClickMenuPopup, this->mainEditorWindow.getMousePos().x, this->mainEditorWindow.getMousePos().y, 150, rightClickMenuPopup->getSize().y);
		}
	});
	
	// Movement tab
	std::shared_ptr<EMPABasedMovementEditorPanel> movementEditorPanel = EMPABasedMovementEditorPanel::create(mainEditorWindow, clipboard);
	movementEditorPanel->connect("EMPAListModified", [this](std::vector<std::shared_ptr<EMPAction>> newActions, float newSumOfDurations) {
		// This shouldn't be undoable here because it's already undoable from EMPABasedMovementEditorPanel.
		// Note: Setting the limits of a SliderWithEditBox to some number and then setting it back does not
		// revert the SliderWithEditBox's value
		this->attackPattern->setActions(newActions);

		onAttackPatternModify.emit(this, this->attackPattern);
	});
	movementEditorPanel->setActions(this->attackPattern->getActions());
	tabs->addTab(MOVEMENT_TAB_NAME, movementEditorPanel, false, false);
}

AttackPatternEditorPanel::~AttackPatternEditorPanel() {
	// TODO
	//levelPack->getOnChange()->sink().disconnect<AttackEditorPanel, &AttackEditorPanel::populatePropertiesUsedByList>(this);
	//mainEditorWindow.getGui()->remove(symbolTableEditorWindow);
}

tgui::Signal& AttackPatternEditorPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onEnemyPhaseBeginEdit.getName())) {
		return onEnemyPhaseBeginEdit;
	} else if (signalName == tgui::toLower(onPlayerBeginEdit.getName())) {
		return onPlayerBeginEdit;
	} else if (signalName == tgui::toLower(onAttackPatternModify.getName())) {
		return onAttackPatternModify;
	}
	return tgui::Panel::getSignal(signalName);
}

void AttackPatternEditorPanel::propagateChangesToChildren() {
	// TODO
}

ValueSymbolTable AttackPatternEditorPanel::getLevelPackObjectSymbolTable() {
	return attackPattern->getSymbolTable();
}

void AttackPatternEditorPanel::onLevelPackChange(LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE type, int id) {
	// Attack patterns can be used only by enemy phases and players, so update usedBy only if the modified enemy phase or player uses this attack pattern
	if ((type == LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ENEMY_PHASE && levelPack->getEnemyPhase(id)->usesAttackPattern(attackPattern->getID()))
		|| (type == LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::PLAYER && levelPack->getPlayer()->usesAttackPattern(attackPattern->getID()))) {
		populatePropertiesUsedByList();
	}
}

void AttackPatternEditorPanel::populatePropertiesUsedByList() {
	usedByIDMap.clear();
	auto usedBy = propertiesPanel->getUsedByPanel();
	usedBy->getListView()->removeAllItems();
	int i = 0;
	if (levelPack->getAttackPatternIsUsedByPlayer(attackPattern->getID())) {
		usedByIDMap[i] = -1;
		usedBy->getListView()->addItem("Player");
		i++;
	}
	auto attackPatternIDs = levelPack->getAttackPatternEnemyUsers(attackPattern->getID());
	for (; i < attackPatternIDs.size(); i++) {
		int id = attackPatternIDs[i];
		usedByIDMap[i] = id;
		usedBy->getListView()->addItem("Enemy phase ID " + std::to_string(id));
	}
	usedBy->onListViewItemsUpdate();
}

void AttackPatternEditorPropertiesPanel::manualUndo() {
	undoStack.undo();
}

void AttackPatternEditorPropertiesPanel::manualRedo() {
	undoStack.redo();
}

AttackPatternEditorPropertiesPanel::AttackPatternEditorPropertiesPanel(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::shared_ptr<EditorAttackPattern> attackPattern, int undoStackSize) 
	: CopyPasteable("EditorAttackPattern"), mainEditorWindow(mainEditorWindow), clipboard(clipboard), attackPattern(attackPattern), undoStack(UndoStack(undoStackSize)) {
	std::shared_ptr<tgui::Label> id = tgui::Label::create();
	std::shared_ptr<tgui::Label> nameLabel = tgui::Label::create();
	name = EditBox::create();
	std::shared_ptr<tgui::Label> usedByLabel = tgui::Label::create();
	usedBy = ListViewScrollablePanel::create();

	id->setText("Attack pattern ID " + std::to_string(attackPattern->getID()));
	nameLabel->setText("Name");
	name->setText(attackPattern->getName());
	usedByLabel->setText("Used by");

	id->setTextSize(TEXT_SIZE);
	nameLabel->setTextSize(TEXT_SIZE);
	name->setTextSize(TEXT_SIZE);
	usedByLabel->setTextSize(TEXT_SIZE);
	usedBy->getListView()->setTextSize(TEXT_SIZE);

	id->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	nameLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(id) + GUI_PADDING_Y);
	name->setPosition(tgui::bindLeft(id), tgui::bindBottom(nameLabel) + GUI_LABEL_PADDING_Y);
	usedByLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(name) + GUI_PADDING_Y);
	usedBy->setPosition(tgui::bindLeft(id), tgui::bindBottom(usedByLabel) + GUI_LABEL_PADDING_Y);

	connect("SizeChanged", [this](sf::Vector2f newSize) {
		name->setSize(newSize.x - GUI_PADDING_X * 2, tgui::bindHeight(name));
		usedBy->setSize(newSize.x - GUI_PADDING_X * 2, newSize.y - tgui::bindTop(usedBy) - GUI_PADDING_Y);
	});

	name->connect("ValueChanged", [this](std::string text) {
		if (ignoreSignals) {
			return;
		}

		std::string oldName = this->attackPattern->getName();

		if (text != oldName) {
			undoStack.execute(UndoableCommand([this, text]() {
				this->attackPattern->setName(text);
				name->setText(text);
				onAttackPatternModify.emit(this);
			}, [this, oldName]() {
				this->attackPattern->setName(oldName);
				name->setText(oldName);
				onAttackPatternModify.emit(this);
			}));
		}
	});

	add(id);
	add(nameLabel);
	add(name);
	add(usedByLabel);
	add(usedBy);
}

std::pair<std::shared_ptr<CopiedObject>, std::string> AttackPatternEditorPropertiesPanel::copyFrom() {
	//TODO
	return std::pair<std::shared_ptr<CopiedObject>, std::string>();
}

std::string AttackPatternEditorPropertiesPanel::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	//TODO
	return std::string();
}

std::string AttackPatternEditorPropertiesPanel::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	//TODO
	return std::string();
}

bool AttackPatternEditorPropertiesPanel::handleEvent(sf::Event event) {
	//TODO
	return false;
}

tgui::Signal& AttackPatternEditorPropertiesPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onAttackPatternModify.getName())) {
		return onAttackPatternModify;
	}
	return tgui::Panel::getSignal(signalName);
}

void AttackPatternEditorPropertiesPanel::manualPaste() {
	//TODO
}

std::shared_ptr<ListViewScrollablePanel> AttackPatternEditorPropertiesPanel::getUsedByPanel() {
	return usedBy;
}
