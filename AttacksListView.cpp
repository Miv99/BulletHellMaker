#include "AttacksListView.h"
#include "EditorWindow.h"

// Parameters: ID and name
const std::string AttacksListView::SAVED_ATTACK_ITEM_FORMAT = "[%d] %s";
const std::string AttacksListView::UNSAVED_ATTACK_ITEM_FORMAT = "*[%d] %s";

AttacksListView::AttacksListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize) : ListViewScrollablePanel(), CopyPasteable("EditorAttack"), levelPack(nullptr),
mainEditorWindow(mainEditorWindow), clipboard(clipboard), undoStack(UndoStack(undoStackSize)) {
	getListView()->setMultiSelect(true);
}

std::shared_ptr<CopiedObject> AttacksListView::copyFrom() {
	// Copy every selected EditorAttack
	std::set<size_t> selectedIndices = listView->getSelectedItemIndices();
	if (selectedIndices.size() > 0) {
		std::vector<std::shared_ptr<EditorAttack>> attacks;
		auto& unsavedAttacks = mainEditorWindow.getUnsavedAttacks();
		for (int selectedIndex : selectedIndices) {
			int id = getAttackIDFromIndex(selectedIndex);
			
			if (unsavedAttacks.count(id) > 0) {
				attacks.push_back(unsavedAttacks[id]);
			} else {
				attacks.push_back(levelPack->getAttack(id));
			}
		}
		return std::make_shared<CopiedEditorAttack>(getID(), attacks);
	}
	return nullptr;
}

void AttacksListView::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	// Create some new EditorAttacks as clones of the copied EditorAttacks
	auto derived = std::static_pointer_cast<CopiedEditorAttack>(pastedObject);
	if (derived) {
		std::set<int> newAttacksIDs = levelPack->getNextAttackIDs(derived->getAttacksCount());
		undoStack.execute(UndoableCommand([this, derived]() {
			for (auto attack : derived->getAttacks()) {
				// Change the ID to be the LevelPack's next attack ID
				int id = levelPack->getNextAttackID();
				attack->setID(id);

				// Create the attack
				levelPack->updateAttack(std::make_shared<EditorAttack>(attack));
			}
			reload();

		}, [this, newAttacksIDs]() {
			for (int id : newAttacksIDs) {
				levelPack->deleteAttack(id);
			}
			reload();
		}));

		// Select the new attack(s) in the list view
		std::set<size_t> indicesInListView;
		for (auto id : newAttacksIDs) {
			indicesInListView.insert(getIndexFromAttackID(id));
		}
		listView->setSelectedItems(indicesInListView);

		// If just one attack, open the attack too
		if (derived->getAttacksCount() == 1) {
			mainEditorWindow.openLeftPanelAttack(*newAttacksIDs.begin());
		}
	}
}

void AttacksListView::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	// Paste the new EditorAttacks such that they overwrite the selected EditorAttacks
	std::set<size_t> selectedIndices = listView->getSelectedItemIndices();
	auto derived = std::static_pointer_cast<CopiedEditorAttack>(pastedObject);
	if (selectedIndices.size() > 0 && derived) {
		std::vector<std::shared_ptr<EditorAttack>> copiedAttacks = derived->getAttacks();
		std::vector<std::shared_ptr<EditorAttack>> newAttacks;

		int i = 0;
		for (int selectedIndex : selectedIndices) {
			int id = getAttackIDFromIndex(selectedIndex);

			std::shared_ptr<EditorAttack> newAttack = std::make_shared<EditorAttack>(copiedAttacks[i % copiedAttacks.size()]);
			// Set the ID of the EditorAttack that's overwriting the old to be the old one's ID
			newAttack->setID(id);
			newAttacks.push_back(newAttack);

			i++;
		}

		// See "Why paste2 in AttacksListView can't be undoable" in personal notes for explanation on why this can't be undoable
		mainEditorWindow.promptConfirmation("Overwrite the selected attack(s) with the copied attack(s)? This will reload their tabs if they are currently open.", newAttacks)->sink()
			.connect<AttacksListView, &AttacksListView::onPasteIntoConfirmation>(this);
	}
}

bool AttacksListView::handleEvent(sf::Event event) {
	if (event.type == sf::Event::KeyPressed) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::Z) {
				undoStack.undo();
				return true;
			} else if (event.key.code == sf::Keyboard::Y) {
				undoStack.redo();
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
			} else if (event.key.code == sf::Keyboard::S) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
					manualSaveAll();
				} else {
					manualSave();
				}
				return true;
			} else if (event.key.code == sf::Keyboard::A) {
				manualSelectAll();
				return true;
			}
		}
		if (event.key.code == sf::Keyboard::Delete) {
			manualDelete();
		}
	}
	return false;
}

void AttacksListView::setLevelPack(LevelPack * levelPack) {
	this->levelPack = levelPack;
}

void AttacksListView::reload() {
	std::set<size_t> selectedIndices = listView->getSelectedItemIndices();
	std::set<int> selectedIDs;
	for (auto it = selectedIndices.begin(); it != selectedIndices.end(); it++) {
		selectedIDs.insert(getAttackIDFromIndex(*it));
	}

	attackIDToAttacksListViewIndexMap.clear();
	attacksListViewIndexToAttackIDMap.clear();
	listView->removeAllItems();

	auto& unsavedAttacks = mainEditorWindow.getUnsavedAttacks();

	int i = 0;
	for (auto it = levelPack->getAttackIteratorBegin(); it != levelPack->getAttackIteratorEnd(); it++) {
		// If the attack is in unsavedAttacks, signify it is unsaved with an asterisk
		if (unsavedAttacks.count(it->first) > 0) {
			listView->addItem(format(UNSAVED_ATTACK_ITEM_FORMAT, it->second->getID(), unsavedAttacks[it->first]->getName().c_str()));
		} else {
			listView->addItem(format(SAVED_ATTACK_ITEM_FORMAT, it->second->getID(), it->second->getName().c_str()));
		}
		attackIDToAttacksListViewIndexMap[it->second->getID()] = i;
		attacksListViewIndexToAttackIDMap[i] = it->second->getID();
		i++;
	}

	// Select the old IDs if they still exist
	std::set<size_t> newSelectedIndices;
	for (auto it = selectedIDs.begin(); it != selectedIDs.end(); it++) {
		if (attackIDToAttacksListViewIndexMap.count(*it) > 0) {
			newSelectedIndices.insert(attackIDToAttacksListViewIndexMap[*it]);
		}
	}

	listView->setSelectedItems(newSelectedIndices);

	onListViewItemsUpdate();
}

int AttacksListView::getAttackIDFromIndex(int index) {
	return attacksListViewIndexToAttackIDMap[index];
}

int AttacksListView::getIndexFromAttackID(int attackID) {
	return attackIDToAttacksListViewIndexMap[attackID];
}

UndoStack& AttacksListView::getUndoStack() {
	return undoStack;
}


void AttacksListView::manualCopy() {
	clipboard.copy(this);
}

void AttacksListView::manualPaste() {
	clipboard.paste(this);
}

void AttacksListView::manualPaste2() {
	clipboard.paste2(this);
}

void AttacksListView::manualDelete() {
	auto selectedIndices = getListView()->getSelectedItemIndices();
	if (selectedIndices.size() > 0) {
		auto& unsavedAttacks = mainEditorWindow.getUnsavedAttacks();
		// EditorAttack and whether it was in unsavedAttacks
		std::vector<std::pair<std::shared_ptr<EditorAttack>, bool>> deletedAttacks;
		for (int index : selectedIndices) {
			int id = mainEditorWindow.getAttacksListView()->getAttackIDFromIndex(index);

			if (unsavedAttacks.count(id) > 0) {
				deletedAttacks.push_back(std::make_pair(std::make_shared<EditorAttack>(unsavedAttacks[id]), true));
			} else {
				deletedAttacks.push_back(std::make_pair(std::make_shared<EditorAttack>(levelPack->getAttack(id)), false));
			}
		}

		undoStack.execute(UndoableCommand([this, selectedIndices]() {
			auto& unsavedAttacks = mainEditorWindow.getUnsavedAttacks();

			for (int index : selectedIndices) {
				int id = mainEditorWindow.getAttacksListView()->getAttackIDFromIndex(index);

				if (unsavedAttacks.count(id) > 0) {
					unsavedAttacks.erase(id);
				}
				levelPack->deleteAttack(id);

				mainEditorWindow.reloadAttackTab(id);
			}

			mainEditorWindow.getAttacksListView()->reload();
		}, [this, deletedAttacks]() {
			auto& unsavedAttacks = mainEditorWindow.getUnsavedAttacks();

			for (std::pair<std::shared_ptr<EditorAttack>, bool> pair : deletedAttacks) {
				levelPack->updateAttack(pair.first);
				// The attack was originally unsaved, so add it back to unsavedAttacks
				if (pair.second) {
					unsavedAttacks[pair.first->getID()] = pair.first;
				}
			}

			mainEditorWindow.getAttacksListView()->reload();
		}));
	}
}

void AttacksListView::manualSave() {
	auto selectedIndices = getListView()->getSelectedItemIndices();
	if (selectedIndices.size() > 0) {
		auto& unsavedAttacks = mainEditorWindow.getUnsavedAttacks();
		for (int index : selectedIndices) {
			int id = mainEditorWindow.getAttacksListView()->getAttackIDFromIndex(index);

			if (unsavedAttacks.count(id) > 0) {
				levelPack->updateAttack(unsavedAttacks[id]);
				unsavedAttacks.erase(id);
			}
			// Do nothing if the attack doesn't have any unsaved changes
		}
	}
	mainEditorWindow.getAttacksListView()->reload();
}

void AttacksListView::manualSaveAll() {
	auto oldSelectedIndices = getListView()->getSelectedItemIndices();
	manualSelectAll();
	manualSave();
	getListView()->setSelectedItems(oldSelectedIndices);
}

void AttacksListView::manualSelectAll() {
	auto attacksListView = mainEditorWindow.getAttacksListView()->getListView();
	int count = attacksListView->getItemCount();
	std::set<size_t> selected;
	for (int i = 0; i < count; i++) {
		selected.insert(i);
	}
	attacksListView->setSelectedItems(selected);
}

void AttacksListView::onPasteIntoConfirmation(bool confirmed, std::vector<std::shared_ptr<EditorAttack>> newAttacks) {
	if (confirmed) {
		mainEditorWindow.overwriteAttacks(newAttacks, &undoStack);
		reload();
	}
}
