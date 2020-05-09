#include "AttackEditorPanel.h"

const std::string AttackEditorPanel::PROPERTIES_TAB_NAME = "Atk. Properties";
const std::string AttackEditorPanel::EMP_TAB_NAME_FORMAT = "EMP %d";

AttackEditorPanel::AttackEditorPanel(EditorWindow& parentWindow, LevelPack& levelPack, SpriteLoader& spriteLoader, std::shared_ptr<EditorAttack> attack, int undoStackSize) : parentWindow(parentWindow), levelPack(levelPack), spriteLoader(spriteLoader), undoStack(UndoStack(undoStackSize)), attack(attack) {
	tabs = TabsWithPanel::create(parentWindow);
	tabs->setPosition(0, 0);
	tabs->setSize("100%", "100%");
	add(tabs);

	{
		// Properties
		std::shared_ptr<tgui::ScrollablePanel> properties = tgui::ScrollablePanel::create();
		std::shared_ptr<tgui::Label> id = tgui::Label::create();
		std::shared_ptr<tgui::Label> nameLabel = tgui::Label::create();
		std::shared_ptr<EditBox> name = EditBox::create();
		std::shared_ptr<tgui::Label> usedByLabel = tgui::Label::create();
		usedBy = ListViewScrollablePanel::create();

		id->setText("Attack ID " + std::to_string(attack->getID()));
		nameLabel->setText("Name");
		name->setText(attack->getName());
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

		name->setSize(tgui::bindWidth(properties) - GUI_PADDING_X * 2, tgui::bindHeight(name));
		usedBy->setSize(tgui::bindWidth(properties) - GUI_PADDING_X * 2, tgui::bindHeight(properties) - tgui::bindTop(usedBy) - GUI_PADDING_Y);

		name->connect("ValueChanged", [this, name](std::string text) {
			std::string oldName = this->attack->getName();
			undoStack.execute(UndoableCommand([this, name, text]() {
				this->attack->setName(text);
				name->setText(text);
				onAttackModify.emit(this, this->attack);
			}, [this, name, oldName]() {
				this->attack->setName(oldName);
				name->setText(oldName);
				onAttackModify.emit(this, this->attack);
			}));
		});

		properties->add(id);
		properties->add(nameLabel);
		properties->add(name);
		properties->add(usedByLabel);
		properties->add(usedBy);
		tabs->addTab(PROPERTIES_TAB_NAME, properties);

		// Populate the usedBy list when the level pack is changed
		levelPack.getOnChange()->sink().connect<AttackEditorPanel, &AttackEditorPanel::populatePropertiesUsedByList>(this);
		// Initial population
		populatePropertiesUsedByList();

		usedBy->getListView()->connect("DoubleClicked", [&](int index) {
			if (usedByIDMap.count(index) > 0) {
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
			usedBy->getListView()->connect("RightClicked", [this, rightClickMenuPopup](int index) {
				if (this->usedByIDMap.count(index) > 0) {
					this->usedByRightClickedAttackPatternID = usedByIDMap[index];
					// Open right click menu
					this->parentWindow.addPopupWidget(rightClickMenuPopup, this->parentWindow.getMousePos().x, this->parentWindow.getMousePos().y, 150, rightClickMenuPopup->getSize().y);
				}
			});
		}
	}
	{
		// EMP tree view
		std::shared_ptr<tgui::Panel> emps = tgui::Panel::create();
		std::shared_ptr<tgui::Label> empsTreeViewLabel = tgui::Label::create();
		empsTreeView = tgui::TreeView::create();

		empsTreeViewLabel->setText("Movable points");

		empsTreeViewLabel->setTextSize(TEXT_SIZE);
		empsTreeView->setTextSize(TEXT_SIZE);

		empsTreeViewLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
		empsTreeView->setPosition(tgui::bindLeft(empsTreeViewLabel), tgui::bindBottom(empsTreeViewLabel) + GUI_LABEL_PADDING_Y);
		empsTreeView->setSize(tgui::bindWidth(emps) - GUI_PADDING_X * 2, tgui::bindHeight(emps) - tgui::bindTop(empsTreeView) - GUI_PADDING_Y);

		emps->add(empsTreeViewLabel);
		emps->add(empsTreeView);
		tabs->addTab("MPs", emps, false);

		{
			// Right click menu for empsTreeView
			auto rightClickMenuPopup = createMenuPopup({
				std::make_pair("Edit", [this]() {
					this->openEMPTab(empRightClickedNodeHierarchy);
				})
			});
			empsTreeView->connect("RightClicked", [this, rightClickMenuPopup](std::vector<sf::String> nodeHierarchy) {
				if (nodeHierarchy.size() > 0) {
					this->empRightClickedNodeHierarchy = nodeHierarchy;
					// Open right click menu
					this->parentWindow.addPopupWidget(rightClickMenuPopup, this->parentWindow.getMousePos().x, this->parentWindow.getMousePos().y, 150, rightClickMenuPopup->getSize().y);
				}
			});
		}
	}
}

AttackEditorPanel::~AttackEditorPanel() {
	levelPack.getOnChange()->sink().disconnect<AttackEditorPanel, &AttackEditorPanel::populatePropertiesUsedByList>(this);
}

bool AttackEditorPanel::handleEvent(sf::Event event) {
	if (tabs->handleEvent(event)) {
		return true;
	}
	if (event.type == sf::Event::KeyPressed) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::Z) {
				undoStack.undo();
				return true;
			} else if (event.key.code == sf::Keyboard::Y) {
				undoStack.redo();
				return true;
			}
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

void AttackEditorPanel::openEMPTab(std::vector<sf::String> empHierarchy) {
	// Open the tab in mainPanel
	int empID = getEMPIDFromTreeViewText(empHierarchy[empHierarchy.size() - 1]);
	if (tabs->hasTab(format(EMP_TAB_NAME_FORMAT, empID))) {
		// Tab already exists, so just select it
		tabs->selectTab(format(EMP_TAB_NAME_FORMAT, empID));
	} else {
		// Create the tab
		std::shared_ptr<EditorMovablePointPanel> empEditorPanel = EditorMovablePointPanel::create(parentWindow, levelPack, spriteLoader, attack->searchEMP(empID));
		empEditorPanel->connect("EMPModified", [&](std::shared_ptr<EditorMovablePoint> emp) {
			populateEMPsTreeView();
			onAttackModify.emit(this, this->attack);
		});
		tabs->addTab(format(EMP_TAB_NAME_FORMAT, empID), empEditorPanel, true, true);
	}
}

void AttackEditorPanel::populatePropertiesUsedByList() {
	usedByIDMap.clear();
	usedBy->getListView()->removeAllItems();
	auto attackPatternIDs = levelPack.getAttackUsers(attack->getID());
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

sf::String AttackEditorPanel::getEMPTextInTreeView(const EditorMovablePoint& emp) {
	std::string bulletStr = emp.getIsBullet() ? "[X]" : "[-]";
	std::string idStr = "[" + std::to_string(emp.getID()) + "]";
	return bulletStr + " " + idStr;
}

int AttackEditorPanel::getEMPIDFromTreeViewText(std::string text) {
	return std::stoi(text.substr(5, text.length() - 5));
}