#include "AttacksListView.h"
#include "EditorWindow.h"

AttacksListView::AttacksListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard) : ListViewScrollablePanel(), CopyPasteable("AttacksListView"), levelPack(nullptr), mainEditorWindow(mainEditorWindow), clipboard(clipboard) {
	getListView()->setMultiSelect(true);
}

std::shared_ptr<CopiedObject> AttacksListView::copyFrom() {
	std::set<size_t> selectedIndices = listView->getSelectedItemIndices();
	if (selectedIndices.size() > 0) {
		std::vector<std::shared_ptr<EditorAttack>> attacks;
		auto& unsavedAttacks = mainEditorWindow.getUnsavedAttacks();
		for (int selectedIndex : selectedIndices) {
			// Assume every item name is in format "[id]..."
			std::string name = listView->getItem(selectedIndex);
			int firstBracketIndex = name.find_first_of('[');
			int id = std::stoi(name.substr(firstBracketIndex + 1, name.find_first_of(']') - firstBracketIndex));
			
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
		for (auto attack : derived->getAttacks()) {
			// Change the ID to be the LevelPack's next attack ID
			attack->setID(levelPack->getNextAttackID());
			mainEditorWindow.createAttack(attack);
		}
	}
}

void AttacksListView::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	std::set<size_t> selectedIndices = listView->getSelectedItemIndices();
	auto derived = std::static_pointer_cast<CopiedEditorAttack>(pastedObject);
	if (selectedIndices.size() > 0 && derived) {
		std::vector<std::shared_ptr<EditorAttack>> copiedAttacks = derived->getAttacks();
		std::vector<std::shared_ptr<EditorAttack>> newAttacks;

		int i = 0;
		for (int selectedIndex : selectedIndices) {
			// Assume every item name is in format "[id]..."
			std::string name = listView->getItem(selectedIndex);
			int firstBracketIndex = name.find_first_of('[');
			int id = std::stoi(name.substr(firstBracketIndex + 1, name.find_first_of(']') - firstBracketIndex));

			std::shared_ptr<EditorAttack> newAttack = std::make_shared<EditorAttack>(copiedAttacks[i % copiedAttacks.size()]);
			// Set the ID of the EditorAttack that's overwriting the old to be the old one's ID
			newAttack->setID(id);
			newAttacks.push_back(newAttack);

			i++;
		}

		mainEditorWindow.overwriteAttacks(newAttacks);
		reload();
	}
}

void AttacksListView::setLevelPack(LevelPack * levelPack) {
	this->levelPack = levelPack;
}

void AttacksListView::reload() {
	auto selectedIndices = listView->getSelectedItemIndices();
	attackIDToAttacksListViewIndexMap.clear();
	attacksListViewIndexToAttackIDMap.clear();
	listView->removeAllItems();

	auto& unsavedAttacks = mainEditorWindow.getUnsavedAttacks();

	int i = 0;
	for (auto it = levelPack->getAttackIteratorBegin(); it != levelPack->getAttackIteratorEnd(); it++) {
		// If the attack is in unsavedAttacks, signify it is unsaved with an asterisk
		if (unsavedAttacks.count(it->first) > 0) {
			listView->addItem("*[" + std::to_string(it->second->getID()) + "] " + unsavedAttacks[it->first]->getName());
		} else {
			listView->addItem("[" + std::to_string(it->second->getID()) + "] " + it->second->getName());
		}
		attackIDToAttacksListViewIndexMap[it->second->getID()] = i;
		attacksListViewIndexToAttackIDMap[i] = it->second->getID();
		i++;
	}

	// Prevent selecting indices that are out of bounds
	for (auto it = selectedIndices.begin(); it != selectedIndices.end();) {
		if (*it >= listView->getItemCount()) {
			selectedIndices.erase(it++);
		} else {
			it++;
		}
	}

	listView->setSelectedItems(selectedIndices);

	onListViewItemsUpdate();
}

int AttacksListView::getAttackIDFromIndex(int index) {
	return attacksListViewIndexToAttackIDMap[index];
}

int AttacksListView::getIndexFromAttackID(int attackID) {
	return attackIDToAttacksListViewIndexMap[attackID];
}

CopiedEditorAttack::CopiedEditorAttack(std::string copiedFromID, std::vector<std::shared_ptr<EditorAttack>> attacks) : CopiedObject(copiedFromID) {
	// Deep copy every attack
	for (auto attack : attacks) {
		this->attacks.push_back(std::make_shared<EditorAttack>(attack));
	}
}

std::vector<std::shared_ptr<EditorAttack>> CopiedEditorAttack::getAttacks() {
	return attacks;
}
