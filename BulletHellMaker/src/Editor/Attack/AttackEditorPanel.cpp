#include <Editor/Attack/AttackEditorPanel.h>

#include <Mutex.h>
#include <Editor/Windows/MainEditorWindow.h>
#include <Editor/Util/EditorUtils.h>
#include <Editor/Attack/EditorMovablePointTreePanel.h>
#include <Editor/LevelPackObjectList/LevelPackObjectsListView.h>

const std::string AttackEditorPanel::PROPERTIES_TAB_NAME = "Atk. Properties";
const std::string AttackEditorPanel::EMP_TAB_NAME_FORMAT = "MP %d";

AttackEditorPanel::AttackEditorPanel(MainEditorWindow& mainEditorWindow, std::shared_ptr<LevelPack> levelPack, SpriteLoader& spriteLoader, 
	Clipboard& clipboard, std::shared_ptr<EditorAttack> attack) 
	: mainEditorWindow(mainEditorWindow), levelPack(levelPack),
	spriteLoader(spriteLoader), clipboard(clipboard), attack(attack) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	tabs = TabsWithPanel::create(mainEditorWindow);
	tabs->setPosition(0, 0);
	tabs->setSize("100%", "100%");
	add(tabs);

	// Properties
	propertiesPanel = AttackEditorPropertiesPanel::create(mainEditorWindow, clipboard, attack);
	propertiesPanel->connect("AttackModified", [this]() {
		onAttackModify.emit(this, this->attack);
	});
	tabs->addTab(PROPERTIES_TAB_NAME, propertiesPanel);

	levelPack->getOnChange()->sink().connect<AttackEditorPanel, &AttackEditorPanel::onLevelPackChange>(this);
	// Initial population
	populatePropertiesUsedByList();

	propertiesPanel->getUsedByPanel()->getListView()->connect("DoubleClicked", [&](int index) {
		if (usedByIDMap.find(index) != usedByIDMap.end()) {
			onAttackPatternBeginEdit.emit(this, usedByIDMap[index]);
		}
	});

	{
		// Right click menu for usedBy
		auto rightClickMenuPopup = createMenuPopup({
			std::make_pair("Edit", [this]() {
				this->onAttackPatternBeginEdit.emit(this, usedByRightClickedAttackPatternID);
			})
		});
		propertiesPanel->getUsedByPanel()->getListView()->connect("RightClicked", [this, rightClickMenuPopup](int index) {
			if (this->usedByIDMap.find(index) != this->usedByIDMap.end()) {
				this->usedByRightClickedAttackPatternID = usedByIDMap[index];
				// Open right click menu
				this->mainEditorWindow.addPopupWidget(rightClickMenuPopup, this->mainEditorWindow.getMousePos().x, this->mainEditorWindow.getMousePos().y, 150, rightClickMenuPopup->getSize().y);
			}
		});
	}

	// EMP tree view
	std::shared_ptr<EditorMovablePointTreePanel> emps = EditorMovablePointTreePanel::create(*this, clipboard, attack);
	emps->connect("EMPModified", [this](std::shared_ptr<EditorMovablePoint> modifiedEMP) {
		populateEMPsTreeView();

		// Reload EMP tab of the modifiedEMP and all its children
		if (modifiedEMP) {
			std::vector<int> modifiedEmpIDs = modifiedEMP->getChildrenIDs();
			reloadEMPTab(modifiedEMP->getID());
			for (auto modifiedEmpID : modifiedEmpIDs) {
				reloadEMPTab(modifiedEmpID);
			}
		}

		onAttackModify.emit(this, this->attack);
	});
	emps->connect("MainEMPChildDeleted", [this](int empID) {
		// Remove the EMP tab if it exists
		std::string tabName = format(EMP_TAB_NAME_FORMAT, empID);
		if (tabs->hasTab(tabName)) {
			tabs->removeTab(tabName);
		}
	});
	empsTreeView = emps->getEmpsTreeView();
	tabs->addTab("MP Tree", emps, false);

	// Right click menu for empsTreeView
	auto rightClickMenuPopup = createMenuPopup({
		// The RightClicked signal defined below guarantees that whatever node is right-clicked in emps's tree view
		// will be selected at the time this 

		std::make_pair("Open", [this]() {
			this->openEMPTab(empsTreeView->getSelectedItem());
		}),
		std::make_pair("Create child", [this, emps]() {
			emps->createEMP(empsTreeView->getSelectedItem());
		}),
		std::make_pair("Copy", [this, emps]() {
			emps->manualCopy();
		}),
		std::make_pair("Paste", [this, emps]() {
			emps->manualPaste();
		}),
		std::make_pair("Paste (override this)", [this, emps]() {
			emps->manualPaste2();
		}),
		std::make_pair("Delete", [this, emps]() {
			emps->manualDelete();
		})
	});
	empsTreeView->connect("RightClicked", [this, rightClickMenuPopup](std::vector<sf::String> nodeHierarchy) {
		if (nodeHierarchy.size() > 0) {
			// Select item manually here because right-clicking tgui::TreeView doesn't select the item automatically
			empsTreeView->selectItem(nodeHierarchy);
			// Open right click menu
			this->mainEditorWindow.addPopupWidget(rightClickMenuPopup, this->mainEditorWindow.getMousePos().x, this->mainEditorWindow.getMousePos().y, 150, rightClickMenuPopup->getSize().y);
		}
	});

	symbolTableEditorWindow = ChildWindow::create();
	symbolTableEditor = ValueSymbolTableEditor::create(false, true);
	symbolTableEditorWindow->setKeepInParent(false);
	symbolTableEditorWindow->add(symbolTableEditor);
	symbolTableEditorWindow->setSize("50%", "50%");
	symbolTableEditorWindow->setTitle("Attack ID " + std::to_string(attack->getID()) + " Variables");
	symbolTableEditorWindow->setFallbackEventHandler([this](sf::Event event) {
		return symbolTableEditor->handleEvent(event);
	});
	symbolTableEditor->connect("ValueChanged", [this](ValueSymbolTable table) {
		this->attack->setSymbolTable(table);
		onChange(table);
		onAttackModify.emit(this, this->attack);
	});

	populateEMPsTreeView();
	updateSymbolTables({ });
}

AttackEditorPanel::~AttackEditorPanel() {
	levelPack->getOnChange()->sink().disconnect<AttackEditorPanel, &AttackEditorPanel::onLevelPackChange>(this);
	mainEditorWindow.removeChildWindow(symbolTableEditorWindow);
}

bool AttackEditorPanel::handleEvent(sf::Event event) {
	if (tabs->handleEvent(event)) {
		return true;
	}
	if (event.type == sf::Event::KeyPressed) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::S) {
				manualSave();
				return true;
			}
		} else if (event.key.code == sf::Keyboard::V) {
			mainEditorWindow.addChildWindow(symbolTableEditorWindow);
			return true;
		}
	}
	return false;
}

tgui::Signal& AttackEditorPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onAttackPatternBeginEdit.getName())) {
		return onAttackPatternBeginEdit;
	} else if (signalName == tgui::toLower(onAttackModify.getName())) {
		return onAttackModify;
	}
	return tgui::Panel::getSignal(signalName);
}

void AttackEditorPanel::propagateChangesToChildren() {
	symbolTableEditor->setSymbolTablesHierarchy(symbolTables);

	auto tabNames = tabs->getTabNames();
	// Start at the index where EMP tabs start
	for (int i = 2; i < tabNames.size(); i++) {
		std::dynamic_pointer_cast<EditorMovablePointPanel>(tabs->getTab(tabNames[i]))->updateSymbolTables(symbolTables);
	}
}

ValueSymbolTable AttackEditorPanel::getLevelPackObjectSymbolTable() {
	return attack->getSymbolTable();
}

void AttackEditorPanel::openEMPTab(std::vector<sf::String> empHierarchy) {
	openEMPTab(getEMPIDFromTreeViewText(empHierarchy[empHierarchy.size() - 1]));
}

void AttackEditorPanel::openEMPTab(int empID) {
	// Open the tab in mainPanel
	if (tabs->hasTab(format(EMP_TAB_NAME_FORMAT, empID))) {
		// Tab already exists, so just select it
		tabs->selectTab(format(EMP_TAB_NAME_FORMAT, empID));
	} else {
		// Create the tab
		std::shared_ptr<EditorMovablePointPanel> empEditorPanel = EditorMovablePointPanel::create(mainEditorWindow, levelPack, spriteLoader, clipboard, attack->searchEMP(empID));
		empEditorPanel->updateSymbolTables(symbolTables);
		empEditorPanel->connect("EMPModified", [&](std::shared_ptr<EditorMovablePoint> emp) {
			populateEMPsTreeView();
			onAttackModify.emit(this, this->attack);
		});
		tabs->addTab(format(EMP_TAB_NAME_FORMAT, empID), empEditorPanel, true, true);
	}
}

void AttackEditorPanel::onLevelPackChange(LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE type, int id) {
	// Attacks can be used only by attack patterns, so update usedBy only if the modified attack pattern uses this attack
	if (type == LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE::ATTACK_PATTERN && levelPack->getAttackPattern(id)->usesAttack(attack->getID())) {
		populatePropertiesUsedByList();
	}
}

void AttackEditorPanel::populatePropertiesUsedByList() {
	usedByIDMap.clear();
	auto usedBy = propertiesPanel->getUsedByPanel();
	usedBy->getListView()->removeAllItems();
	auto attackPatternIDs = levelPack->getAttackUsers(attack->getID());
	for (int i = 0; i < attackPatternIDs.size(); i++) {
		int id = attackPatternIDs[i];
		usedByIDMap[i] = id;
		usedBy->getListView()->addItem("Attack pattern ID " + std::to_string(id));
	}
	usedBy->onListViewItemsUpdate();
}

void AttackEditorPanel::populateEMPsTreeView() {
	empsTreeView->removeAllItems();
	std::vector<std::vector<sf::String>> paths = attack->getMainEMP()->generateTreeViewEmpHierarchy(getEMPTextInTreeView, {});
	for (std::vector<sf::String> path : paths) {
		empsTreeView->addItem(path);
	}
}

void AttackEditorPanel::manualSave() {
	mainEditorWindow.saveAttackChanges(attack->getID());
}

void AttackEditorPanel::reloadEMPTab(int empID) {
	std::string tabName = format(EMP_TAB_NAME_FORMAT, empID);
	// Do nothing if the tab doesn't exist
	if (tabs->hasTab(tabName)) {
		// Remove the tab and then re-insert it at its old index
		bool tabWasSelected = tabs->getSelectedTab() == tabName;
		int tabIndex = tabs->getTabIndex(tabName);
		tabs->removeTab(tabName);

		// Create the tab
		std::shared_ptr<EditorMovablePointPanel> empEditorPanel = EditorMovablePointPanel::create(mainEditorWindow, levelPack, spriteLoader, clipboard, attack->searchEMP(empID));
		empEditorPanel->connect("EMPModified", [&](std::shared_ptr<EditorMovablePoint> emp) {
			populateEMPsTreeView();
			onAttackModify.emit(this, this->attack);
		});
		tabs->insertTab(tabName, empEditorPanel, tabIndex, tabWasSelected, true);
	}
}

sf::String AttackEditorPanel::getEMPTextInTreeView(const EditorMovablePoint& emp) {
	std::string bulletStr = emp.getIsBullet() ? "[X]" : "[-]";
	std::string idStr = "[" + std::to_string(emp.getID()) + "]";
	return bulletStr + " " + idStr;
}

int AttackEditorPanel::getEMPIDFromTreeViewText(std::string text) {
	return std::stoi(text.substr(5, text.length() - 5));
}