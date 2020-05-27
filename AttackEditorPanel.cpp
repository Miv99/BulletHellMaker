#include "AttackEditorPanel.h"

const std::string AttackEditorPanel::PROPERTIES_TAB_NAME = "Atk. Properties";
const std::string AttackEditorPanel::EMP_TAB_NAME_FORMAT = "EMP %d";

AttackEditorPropertiesPanel::AttackEditorPropertiesPanel(MainEditorWindow& mainEditorWindow, Clipboard & clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize) : CopyPasteable("EditorAttack"), mainEditorWindow(mainEditorWindow), clipboard(clipboard), 
attack(attack), undoStack(UndoStack(undoStackSize)) {
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

	connect("SizeChanged", [this, name](sf::Vector2f newSize) {
		name->setSize(newSize.x - GUI_PADDING_X * 2, tgui::bindHeight(name));
		usedBy->setSize(newSize.x - GUI_PADDING_X * 2, newSize.y - tgui::bindTop(usedBy) - GUI_PADDING_Y);
	});

	name->connect("ValueChanged", [this, name](std::string text) {
		std::string oldName = this->attack->getName();

		if (text != oldName) {
			undoStack.execute(UndoableCommand([this, name, text]() {
				this->attack->setName(text);
				name->setText(text);
				onAttackModify.emit(this);
			}, [this, name, oldName]() {
				this->attack->setName(oldName);
				name->setText(oldName);
				onAttackModify.emit(this);
			}));
		}
	});

	add(id);
	add(nameLabel);
	add(name);
	add(usedByLabel);
	add(usedBy);
}

std::shared_ptr<CopiedObject> AttackEditorPropertiesPanel::copyFrom() {
	// Can't copy this widget
	return nullptr;
}

void AttackEditorPropertiesPanel::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	// Same functionality as paste2Into()
	paste2Into(pastedObject);
}

void AttackEditorPropertiesPanel::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	auto derived = std::static_pointer_cast<CopiedEditorAttack>(pastedObject);
	if (derived) {
		std::vector<std::shared_ptr<EditorAttack>> copiedAttacks = derived->getAttacks();
		if (copiedAttacks.size() > 0) {
			std::shared_ptr<EditorAttack> newAttack = std::make_shared<EditorAttack>(attack);
			newAttack->setName(copiedAttacks[0]->getName());

			mainEditorWindow.promptConfirmation("Overwrite this Attack's properties with those of Attack \"" + copiedAttacks[0]->getName() + "\" (ID " + std::to_string(copiedAttacks[0]->getID()) + ")? This action cannot be undone.", newAttack)->sink()
				.connect<AttackEditorPropertiesPanel, &AttackEditorPropertiesPanel::onPasteIntoConfirmation>(this);
		}
	}
}

bool AttackEditorPropertiesPanel::handleEvent(sf::Event event) {
	if (event.type == sf::Event::KeyPressed) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::Z) {
				manualUndo();
				return true;
			} else if (event.key.code == sf::Keyboard::Y) {
				manualRedo();
				return true;
			} else if (event.key.code == sf::Keyboard::V) {
				manualPaste();
				return true;
			}
		}
	}
	return false;
}

tgui::Signal & AttackEditorPropertiesPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onAttackModify.getName())) {
		return onAttackModify;
	}
	return tgui::Panel::getSignal(signalName);
}

void AttackEditorPropertiesPanel::manualPaste() {
	clipboard.paste2(this);
}

std::shared_ptr<ListViewScrollablePanel> AttackEditorPropertiesPanel::getUsedByPanel() {
	return usedBy;
}

void AttackEditorPropertiesPanel::manualUndo() {
	undoStack.undo();
}

void AttackEditorPropertiesPanel::manualRedo() {
	undoStack.redo();
}

void AttackEditorPropertiesPanel::onPasteIntoConfirmation(bool confirmed, std::shared_ptr<EditorAttack> newAttack) {
	if (confirmed) {
		// Overwrite everything in the attack's properties
		mainEditorWindow.overwriteAttacks({ newAttack }, nullptr);
	}
}

AttackEditorPanel::AttackEditorPanel(MainEditorWindow& mainEditorWindow, LevelPack& levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize) : mainEditorWindow(mainEditorWindow), levelPack(levelPack), 
spriteLoader(spriteLoader), clipboard(clipboard), undoStack(UndoStack(undoStackSize)), attack(attack) {
	tabs = TabsWithPanel::create(mainEditorWindow);
	tabs->setPosition(0, 0);
	tabs->setSize("100%", "100%");
	add(tabs);

	{
		// Properties
		properties = AttackEditorPropertiesPanel::create(mainEditorWindow, clipboard, attack);
		properties->connect("AttackModified", [this]() {
			onAttackModify.emit(this, this->attack);
		});
		tabs->addTab(PROPERTIES_TAB_NAME, properties);

		// Populate the usedBy list when the level pack is changed
		levelPack.getOnChange()->sink().connect<AttackEditorPanel, &AttackEditorPanel::populatePropertiesUsedByList>(this);
		// Initial population
		populatePropertiesUsedByList();

		properties->getUsedByPanel()->getListView()->connect("DoubleClicked", [&](int index) {
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
			properties->getUsedByPanel()->getListView()->connect("RightClicked", [this, rightClickMenuPopup](int index) {
				if (this->usedByIDMap.count(index) > 0) {
					this->usedByRightClickedAttackPatternID = usedByIDMap[index];
					// Open right click menu
					this->mainEditorWindow.addPopupWidget(rightClickMenuPopup, this->mainEditorWindow.getMousePos().x, this->mainEditorWindow.getMousePos().y, 150, rightClickMenuPopup->getSize().y);
				}
			});
		}
	}
	{
		// EMP tree view
		std::shared_ptr<EditorMovablePointTreePanel> emps = EditorMovablePointTreePanel::create(clipboard, attack);
		emps->connect("MainEMPModified", [this]() {
			populateEMPsTreeView();
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
					this->mainEditorWindow.addPopupWidget(rightClickMenuPopup, this->mainEditorWindow.getMousePos().x, this->mainEditorWindow.getMousePos().y, 150, rightClickMenuPopup->getSize().y);
				}
			});
		}
	}

	populateEMPsTreeView();
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
			} else if (event.key.code == sf::Keyboard::S) {
				manualSave();
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
		std::shared_ptr<EditorMovablePointPanel> empEditorPanel = EditorMovablePointPanel::create(mainEditorWindow, levelPack, spriteLoader, attack->searchEMP(empID));
		empEditorPanel->connect("EMPModified", [&](std::shared_ptr<EditorMovablePoint> emp) {
			populateEMPsTreeView();
			onAttackModify.emit(this, this->attack);
		});
		tabs->addTab(format(EMP_TAB_NAME_FORMAT, empID), empEditorPanel, true, true);
	}
}

void AttackEditorPanel::populatePropertiesUsedByList() {
	usedByIDMap.clear();
	auto usedBy = properties->getUsedByPanel();
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

void AttackEditorPanel::manualSave() {
	auto& unsavedAttacks = mainEditorWindow.getUnsavedAttacks();
	int id = attack->getID();

	if (unsavedAttacks.count(id) > 0) {
		levelPack.updateAttack(unsavedAttacks[id]);
		unsavedAttacks.erase(id);
	}
	// Do nothing if the attack doesn't have any unsaved changes

	mainEditorWindow.getAttacksListView()->reload();
}

sf::String AttackEditorPanel::getEMPTextInTreeView(const EditorMovablePoint& emp) {
	std::string bulletStr = emp.getIsBullet() ? "[X]" : "[-]";
	std::string idStr = "[" + std::to_string(emp.getID()) + "]";
	return bulletStr + " " + idStr;
}

int AttackEditorPanel::getEMPIDFromTreeViewText(std::string text) {
	return std::stoi(text.substr(5, text.length() - 5));
}

EditorMovablePointTreePanel::EditorMovablePointTreePanel(Clipboard & clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize) : CopyPasteable("EditorMovablePoint"), clipboard(clipboard),
attack(attack), undoStack(UndoStack(undoStackSize)) {
	empsTreeView = tgui::TreeView::create();
	std::shared_ptr<tgui::Label> empsTreeViewLabel = tgui::Label::create();

	empsTreeViewLabel->setText("Movable points");

	empsTreeViewLabel->setTextSize(TEXT_SIZE);
	empsTreeView->setTextSize(TEXT_SIZE);

	empsTreeViewLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	empsTreeView->setPosition(tgui::bindLeft(empsTreeViewLabel), tgui::bindBottom(empsTreeViewLabel) + GUI_LABEL_PADDING_Y);
	connect("SizeChanged", [&](sf::Vector2f newSize) {
		empsTreeView->setSize(newSize.x - GUI_PADDING_X * 2, newSize.y - tgui::bindTop(empsTreeView) - GUI_PADDING_Y);
	});

	add(empsTreeViewLabel);
	add(empsTreeView);
}

std::shared_ptr<CopiedObject> EditorMovablePointTreePanel::copyFrom() {
	return std::shared_ptr<CopiedObject>();
}

void EditorMovablePointTreePanel::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
}

void EditorMovablePointTreePanel::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
}

bool EditorMovablePointTreePanel::handleEvent(sf::Event event) {
	if (event.type == sf::Event::KeyPressed) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::Z) {
				manualUndo();
				return true;
			} else if (event.key.code == sf::Keyboard::Y) {
				manualRedo();
				return true;
			} else if (event.key.code == sf::Keyboard::C) {
				manualCopy();
				return true;
			} else if (event.key.code == sf::Keyboard::V) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
					manualPaste2();
				} else {
					manualPaste();
				}
				return true;
			}
		}
		if (event.key.code == sf::Keyboard::Delete) {
			manualDelete();
		}
	}
	return false;
}

tgui::Signal & EditorMovablePointTreePanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onMainEMPModify.getName())) {
		return onMainEMPModify;
	} else if (signalName == tgui::toLower(onMainEMPChildDeletion.getName())) {
		return onMainEMPChildDeletion;
	}
	return tgui::Panel::getSignal(signalName);
}

std::shared_ptr<tgui::TreeView> EditorMovablePointTreePanel::getEmpsTreeView() {
	return empsTreeView;
}

void EditorMovablePointTreePanel::manualDelete() {
	auto selected = empsTreeView->getSelectedItem();
	// The hierarchy size must be > 1 because a size of 1 means
	// the main EMP is selected, which should not be able to be deleted
	if (selected.size() > 1) {
		std::shared_ptr<EditorMovablePoint> removedEMP = attack->searchEMP(AttackEditorPanel::getEMPIDFromTreeViewText(selected[selected.size() - 1]));

		undoStack.execute(UndoableCommand([this, selected]() {
			// Remove the EMP from attack
			int empID = AttackEditorPanel::getEMPIDFromTreeViewText(selected[selected.size() - 1]);
			attack->getMainEMP()->removeChild(empID);

			onMainEMPModify.emit(this);
			onMainEMPChildDeletion.emit(this, empID);
		}, [this, selected, removedEMP]() {
			// Add removedEMP back
			int empParentID = AttackEditorPanel::getEMPIDFromTreeViewText(selected[selected.size() - 2]);
			attack->searchEMP(empParentID)->addChild(removedEMP);

			onMainEMPModify.emit(this);
		}));
	}
}

void EditorMovablePointTreePanel::manualUndo() {
	undoStack.undo();
}

void EditorMovablePointTreePanel::manualRedo() {
	undoStack.redo();
}

void EditorMovablePointTreePanel::manualCopy() {
	clipboard.copy(this);
}

void EditorMovablePointTreePanel::manualPaste() {
	clipboard.paste(this);
}

void EditorMovablePointTreePanel::manualPaste2() {
	clipboard.paste2(this);
}
