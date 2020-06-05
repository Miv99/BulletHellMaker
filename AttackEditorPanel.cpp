#include "AttackEditorPanel.h"

const std::string AttackEditorPanel::PROPERTIES_TAB_NAME = "Atk. Properties";
const std::string AttackEditorPanel::EMP_TAB_NAME_FORMAT = "EMP %d";

AttackEditorPropertiesPanel::AttackEditorPropertiesPanel(MainEditorWindow& mainEditorWindow, Clipboard & clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize) : CopyPasteable("EditorAttack"), mainEditorWindow(mainEditorWindow), clipboard(clipboard), 
attack(attack), undoStack(UndoStack(undoStackSize)) {
	std::shared_ptr<tgui::Label> id = tgui::Label::create();
	std::shared_ptr<tgui::Label> nameLabel = tgui::Label::create();
	name = EditBox::create();
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

	connect("SizeChanged", [this](sf::Vector2f newSize) {
		name->setSize(newSize.x - GUI_PADDING_X * 2, tgui::bindHeight(name));
		usedBy->setSize(newSize.x - GUI_PADDING_X * 2, newSize.y - tgui::bindTop(usedBy) - GUI_PADDING_Y);
	});

	name->connect("ValueChanged", [this](std::string text) {
		if (ignoreSignals) {
			return;
		}

		std::string oldName = this->attack->getName();

		if (text != oldName) {
			undoStack.execute(UndoableCommand([this, text]() {
				this->attack->setName(text);
				name->setText(text);
				onAttackModify.emit(this);
			}, [this, oldName]() {
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
	// Paste the first copied EditorAttack to override attack's properties
	auto derived = std::static_pointer_cast<CopiedEditorAttack>(pastedObject);
	if (derived) {
		std::vector<std::shared_ptr<EditorAttack>> copiedAttacks = derived->getAttacks();
		if (copiedAttacks.size() > 0) {
			std::shared_ptr<EditorAttack> newAttack = std::make_shared<EditorAttack>(attack);
			newAttack->setName(copiedAttacks[0]->getName());

			mainEditorWindow.promptConfirmation("Overwrite this attack's properties with the copied attack's properties?", newAttack)->sink()
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
		std::string oldName = attack->getName();
		std::string newName = newAttack->getName();
		undoStack.execute(UndoableCommand([this, newName]() {
			ignoreSignals = true;
			attack->setName(newName);
			this->name->setText(newName);
			ignoreSignals = false;

			onAttackModify.emit(this);
		}, [this, oldName]() {
			attack->setName(oldName);
			this->name->setText(oldName);
			onAttackModify.emit(this);
		}));
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
		tabs->insertTab(format(EMP_TAB_NAME_FORMAT, empID), empEditorPanel, tabIndex, tabWasSelected, true);
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

EditorMovablePointTreePanel::EditorMovablePointTreePanel(AttackEditorPanel& parentAttackEditorPanel, Clipboard & clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize) : CopyPasteable("EditorMovablePoint"), parentAttackEditorPanel(parentAttackEditorPanel), clipboard(clipboard),
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
	auto selected = empsTreeView->getSelectedItem();
	if (selected.size() > 0) {
		return std::make_shared<CopiedEditorMovablePoint>(getID(), attack->searchEMP(parentAttackEditorPanel.getEMPIDFromTreeViewText(selected[selected.size() - 1])));
	}
	return nullptr;
}

void EditorMovablePointTreePanel::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	auto derived = std::static_pointer_cast<CopiedEditorMovablePoint>(pastedObject);
	if (derived) {
		auto selected = empsTreeView->getSelectedItem();
		if (selected.size() > 0) {
			std::shared_ptr<EditorMovablePoint> emp = derived->getEMP();
			emp->onNewParentEditorAttack(attack);

			std::shared_ptr<EditorMovablePoint> selectedEMP = attack->searchEMP(parentAttackEditorPanel.getEMPIDFromTreeViewText(selected[selected.size() - 1]));

			undoStack.execute(UndoableCommand([this, emp, selectedEMP]() {
				// Add the copied EMP as a child of the selected EMP
				selectedEMP->addChild(emp);

				onEMPModify.emit(this, emp);
			}, [this, emp, selectedEMP]() {
				selectedEMP->removeChild(emp->getID());

				onEMPModify.emit(this, emp);
				// Emit onMainEMPChildDeletion for the deleted EMP and all its children
				onMainEMPChildDeletion.emit(this, emp->getID());
				std::vector<int> deletedEmpIDs = emp->getChildrenIDs();
				for (int deletedID : deletedEmpIDs) {
					onMainEMPChildDeletion.emit(this, deletedID);
				}
			}));
		}
	}
}

void EditorMovablePointTreePanel::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	auto derived = std::static_pointer_cast<CopiedEditorMovablePoint>(pastedObject);
	if (derived) {
		auto selected = empsTreeView->getSelectedItem();
		if (selected.size() > 0) {
			// Overwrite the selected EMP with the copied EMP but keep the ID

			std::shared_ptr<EditorMovablePoint> selectedEMP = attack->searchEMP(parentAttackEditorPanel.getEMPIDFromTreeViewText(selected[selected.size() - 1]));
			std::string selectedEMPOldFormat = selectedEMP->format();

			undoStack.execute(UndoableCommand([this, derived, selectedEMP]() {
				std::shared_ptr<EditorMovablePoint> pastedEMP = derived->getEMP();

				// Gather the ID of every EMP that will be deleted
				std::vector<int> deletedEmpIDs = selectedEMP->getChildrenIDs();

				// Do the actual pasting
				int oldID = selectedEMP->getID();
				selectedEMP->load(pastedEMP->format());
				selectedEMP->setID(oldID);

				onEMPModify.emit(this, selectedEMP);
				// Emit onMainEMPChildDeletion for every deleted EMP
				for (int deletedID : deletedEmpIDs) {
					onMainEMPChildDeletion.emit(this, deletedID);
				}
			}, [this, selectedEMP, selectedEMPOldFormat]() {
				// Gather the ID of every EMP that will be deleted
				std::vector<int> deletedEmpIDs = selectedEMP->getChildrenIDs();

				int oldID = selectedEMP->getID();
				selectedEMP->load(selectedEMPOldFormat);
				selectedEMP->setID(oldID);

				onEMPModify.emit(this, selectedEMP);
				// Emit onMainEMPChildDeletion for every deleted EMP
				for (int deletedID : deletedEmpIDs) {
					onMainEMPChildDeletion.emit(this, deletedID);
				}
			}));
		}
	}
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
	if (signalName == tgui::toLower(onEMPModify.getName())) {
		return onEMPModify;
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
		std::shared_ptr<EditorMovablePoint> parentEMP = attack->searchEMP(AttackEditorPanel::getEMPIDFromTreeViewText(selected[selected.size() - 2]));

		undoStack.execute(UndoableCommand([this, removedEMP, parentEMP]() {
			// Remove the EMP from attack
			int empID = removedEMP->getID();
			parentEMP->removeChild(empID);

			// Tab deletion will be taken care of by the onMainEMPChildDeletion signal, so no need to reload any tabs with the onEMPModify signal
			onEMPModify.emit(this, nullptr);
			// Emit onMainEMPChildDeletion for the deleted EMP and all its children
			onMainEMPChildDeletion.emit(this, empID);
			std::vector<int> deletedEmpIDs = removedEMP->getChildrenIDs();
			for (int deletedID : deletedEmpIDs) {
				onMainEMPChildDeletion.emit(this, deletedID);
			}
		}, [this, selected, removedEMP, empParentID = parentEMP->getID()]() {
			// Add removedEMP back
			int empParentID = AttackEditorPanel::getEMPIDFromTreeViewText(selected[selected.size() - 2]);
			attack->searchEMP(empParentID)->addChild(removedEMP);

			// An EMP is being added back, so no need to reload any tabs in the parent AttackEditorPanel
			onEMPModify.emit(this, nullptr);
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