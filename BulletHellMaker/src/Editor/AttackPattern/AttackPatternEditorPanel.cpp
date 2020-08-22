#include <Editor/AttackPattern/AttackPatternEditorPanel.h>

#include <Editor/EditorWindow.h>
#include <Editor/Util/EditorUtils.h>

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