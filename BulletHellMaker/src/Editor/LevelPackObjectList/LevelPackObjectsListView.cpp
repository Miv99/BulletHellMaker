#include <Editor/LevelPackObjectList/LevelPackObjectsListView.h>

#include <Editor/Windows/MainEditorWindow.h>

// Parameters: ID and name
const std::string LevelPackObjectsListView::SAVED_LEVEL_PACK_OBJECT_ITEM_FORMAT = "[%d] %s";
const std::string LevelPackObjectsListView::UNSAVED_LEVEL_PACK_OBJECT_ITEM_FORMAT = "*[%d] %s";

LevelPackObjectsListView::LevelPackObjectsListView(std::string copyPasteableID, MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize) : ListViewScrollablePanel(),
CopyPasteable(copyPasteableID), levelPack(nullptr), mainEditorWindow(mainEditorWindow), clipboard(clipboard), undoStack(UndoStack(undoStackSize)), sortOption(SORT_OPTION::ID) {
	getListView()->setMultiSelect(true);
}

CopyOperationResult LevelPackObjectsListView::copyFrom() {
	// Copy every selected object
	std::set<size_t> selectedIndices = listView->getSelectedItemIndices();
	if (selectedIndices.size() > 0) {
		std::vector<std::shared_ptr<LevelPackObject>> objs;
		std::map<int, std::shared_ptr<LevelPackObject>>& unsavedObjs = getUnsavedLevelPackObjects();
		for (int selectedIndex : selectedIndices) {
			int id = getLevelPackObjectIDFromIndex(selectedIndex);

			if (unsavedObjs.find(id) != unsavedObjs.end()) {
				objs.push_back(unsavedObjs[id]);
			} else {
				objs.push_back(getLevelPackObjectFromLevelPack(id));
			}
		}

		std::string copyMessage;
		if (selectedIndices.size() == 1) {
			copyMessage = "Copied 1 " + getLevelPackObjectDisplayName();
		} else {
			copyMessage = "Copied " + std::to_string(selectedIndices.size()) + " " + getLevelPackObjectDisplayName() + "s";
		}
		return CopyOperationResult(std::make_shared<CopiedLevelPackObject>(getID(), objs), "Copied " + std::to_string(selectedIndices.size()) + " " + getLevelPackObjectDisplayName() + "(s)");
	}
	return CopyOperationResult(nullptr, "");
}


PasteOperationResult LevelPackObjectsListView::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	// Create some new LevelPackObjects as clones of the copied LevelPackObjects

	auto derived = std::static_pointer_cast<CopiedLevelPackObject>(pastedObject);
	if (derived) {
		std::set<int> newObjIDs = getNextLevelPackObjectIDs(derived->getLevelPackObjectsCount());
		undoStack.execute(UndoableCommand([this, derived]() {
			for (auto obj : derived->getLevelPackObjects()) {
				// Change the ID to be the LevelPack's next object ID
				int id = *getNextLevelPackObjectIDs(1).begin();
				obj->setID(id);

				// Create the object
				updateLevelPackObjectInLevelPack(obj->clone());
			}
			reload();

		}, [this, newObjIDs]() {
			for (int id : newObjIDs) {
				deleteLevelPackObjectInLevelPack(id);
			}
			reload();
		}));

		// Select the new attack(s) in the list view
		std::set<size_t> indicesInListView;
		for (auto id : newObjIDs) {
			indicesInListView.insert(getIndexFromLevelPackObjectID(id));
		}
		listView->setSelectedItems(indicesInListView);

		// If just one attack, open the attack too
		if (derived->getLevelPackObjectsCount() == 1) {
			openLevelPackObjectInMainEditorWindow(*newObjIDs.begin());
		}

		if (derived->getLevelPackObjectsCount() == 1) {
			return PasteOperationResult(true, "Pasted 1 " + getLevelPackObjectDisplayName());
		} else {
			return PasteOperationResult(true, "Pasted " + std::to_string(derived->getLevelPackObjectsCount()) + " " + getLevelPackObjectDisplayName() + "s");
		}
	}
	return PasteOperationResult(false, "Type mismatch");
}

PasteOperationResult LevelPackObjectsListView::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	// Paste the new LevelPackObjects such that they overwrite the selected LevelPackObjects

	std::set<size_t> selectedIndices = listView->getSelectedItemIndices();
	auto derived = std::static_pointer_cast<CopiedLevelPackObject>(pastedObject);
	if (selectedIndices.size() > 0 && derived) {
		std::vector<std::shared_ptr<LevelPackObject>> copiedObjs = derived->getLevelPackObjects();
		if (copiedObjs.size() == selectedIndices.size()) {
			std::vector<std::shared_ptr<LevelPackObject>> newObjects;

			int i = 0;
			for (int selectedIndex : selectedIndices) {
				int id = getLevelPackObjectIDFromIndex(selectedIndex);

				std::shared_ptr<LevelPackObject> newObject = copiedObjs[i % copiedObjs.size()]->clone();
				// Set the ID of the LevelPackObject that's overwriting the old to be the old one's ID
				newObject->setID(id);
				newObjects.push_back(newObject);

				i++;
			}

			// See "Why paste2 in LevelPackObjectsListView can't be undoable" in personal notes for explanation on why this can't be undoable
			mainEditorWindow.promptConfirmation(getPasteIntoConfirmationPrompt(), newObjects, this)->sink()
				.connect<LevelPackObjectsListView, &LevelPackObjectsListView::onPasteIntoConfirmation>(this);
			return PasteOperationResult(true, "");
		} else {
			std::string s1;
			if (copiedObjs.size() == 1) {
				s1 = "1 " + getLevelPackObjectDisplayName() + " was";
			} else {
				s1 = std::to_string(copiedObjs.size()) + " " + getLevelPackObjectDisplayName() + "s were";
			}
			std::string s2;
			if (selectedIndices.size() == 1) {
				s2 = "only 1 is";
			} else {
				s2 = std::to_string(selectedIndices.size()) + " are";
			}
			return PasteOperationResult(false, "Size mismatch. " + s1 + " copied but " + s2 + " selected");
		}
	}
	return PasteOperationResult(false, "Type mismatch");
}

bool LevelPackObjectsListView::handleEvent(sf::Event event) {
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

void LevelPackObjectsListView::setLevelPack(LevelPack * levelPack) {
	this->levelPack = levelPack;

	listView->deselectItems();
	listView->removeAllItems();

	addLevelPackObjectsToListView();
}

void LevelPackObjectsListView::reload() {
	std::set<size_t> selectedIndices = listView->getSelectedItemIndices();
	std::set<int> selectedIDs;
	for (auto it = selectedIndices.begin(); it != selectedIndices.end(); it++) {
		selectedIDs.insert(getLevelPackObjectIDFromIndex(*it));
	}

	levelPackObjectIDToListViewIndexMap.clear();
	listViewIndexToLevelPackObjectIDMap.clear();
	listView->removeAllItems();

	// Add the objects back into the list view
	addLevelPackObjectsToListView();

	// Select the old IDs if they still exist
	std::set<size_t> newSelectedIndices;
	for (auto it = selectedIDs.begin(); it != selectedIDs.end(); it++) {
		if (levelPackObjectIDToListViewIndexMap.find(*it) != levelPackObjectIDToListViewIndexMap.end()) {
			newSelectedIndices.insert(levelPackObjectIDToListViewIndexMap[*it]);
		}
	}

	listView->setSelectedItems(newSelectedIndices);

	onListViewItemsUpdate();
}

void LevelPackObjectsListView::cycleSortOption() {
	if (sortOption == SORT_OPTION::ID) {
		sortOption = SORT_OPTION::NAME;
	} else {
		sortOption = SORT_OPTION::ID;
	}
	reload();
}

int LevelPackObjectsListView::getLevelPackObjectIDFromIndex(int index) {
	return listViewIndexToLevelPackObjectIDMap[index];
}

int LevelPackObjectsListView::getIndexFromLevelPackObjectID(int id) {
	return levelPackObjectIDToListViewIndexMap[id];
}

UndoStack& LevelPackObjectsListView::getUndoStack() {
	return undoStack;
}


void LevelPackObjectsListView::manualCopy() {
	clipboard.copy(this);
}

void LevelPackObjectsListView::manualPaste() {
	clipboard.paste(this);
}

void LevelPackObjectsListView::manualPaste2() {
	clipboard.paste2(this);
}

void LevelPackObjectsListView::manualDelete() {
	auto selectedIndices = getListView()->getSelectedItemIndices();
	if (selectedIndices.size() > 0) {
		auto& unsavedObjs = getUnsavedLevelPackObjects();
		// Each pair is the LevelPackObject and whether it was in unsavedObjs
		std::vector<std::pair<std::shared_ptr<LevelPackObject>, bool>> deletedObjs;
		bool atLeastOneObjectInUse = false;
		for (int index : selectedIndices) {
			int id = getLevelPackObjectIDFromIndex(index);

			if (unsavedObjs.find(id) != unsavedObjs.end()) {
				deletedObjs.push_back(std::make_pair(unsavedObjs[id]->clone(), true));
			} else {
				deletedObjs.push_back(std::make_pair(getLevelPackObjectFromLevelPack(id)->clone(), false));
			}

			if (getLevelPackObjectIsInUse(id)) {
				atLeastOneObjectInUse = true;
			}
		}

		if (atLeastOneObjectInUse) {
			mainEditorWindow.promptConfirmation(getDeleteLevelPackObjectsInUseConfirmationPrompt(), deletedObjs, this)->sink()
				.connect<LevelPackObjectsListView, &LevelPackObjectsListView::onDeleteConfirmation>(this);
		} else {
			deleteObjects(selectedIndices, deletedObjs);
		}
	}
}

void LevelPackObjectsListView::manualSave() {
	std::set<size_t> selectedIndices = getListView()->getSelectedItemIndices();
	if (selectedIndices.size() > 0) {
		saveLevelPackObjects(selectedIndices);
		reload();
	}
}

void LevelPackObjectsListView::manualSaveAll() {
	auto oldSelectedIndices = getListView()->getSelectedItemIndices();
	manualSelectAll();
	manualSave();
	getListView()->setSelectedItems(oldSelectedIndices);
}

void LevelPackObjectsListView::manualSelectAll() {
	int count = getListView()->getItemCount();
	std::set<size_t> selected;
	for (int i = 0; i < count; i++) {
		selected.insert(i);
	}
	getListView()->setSelectedItems(selected);
}

AttacksListView::AttacksListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize) : LevelPackObjectsListView("EditorAttack", mainEditorWindow, clipboard, undoStackSize) {
}

void AttacksListView::addLevelPackObjectsToListView() {
	auto& unsavedObjs = getUnsavedLevelPackObjects();
	if (sortOption == SORT_OPTION::ID) {
		for (auto it = levelPack->getAttackIteratorBegin(); it != levelPack->getAttackIteratorEnd(); it++) {
			if (it->first < 0) {
				continue;
			}

			bool objIsUnsaved = unsavedObjs.find(it->first) != unsavedObjs.end();
			addLevelPackObjectToListView(it->first, it->second->getName(), objIsUnsaved, objIsUnsaved ? unsavedObjs[it->first]->getName() : "");
		}
	} else {
		std::vector<std::pair<std::string, int>> objsOrdering;
		for (auto it = levelPack->getAttackIteratorBegin(); it != levelPack->getAttackIteratorEnd(); it++) {
			if (it->first < 0) {
				continue;
			}

			if (unsavedObjs.find(it->first) != unsavedObjs.end()) {
				objsOrdering.push_back(std::make_pair(unsavedObjs[it->first]->getName(), it->first));
			} else {
				objsOrdering.push_back(std::make_pair(it->second->getName(), it->first));
			}
		}
		std::sort(objsOrdering.begin(), objsOrdering.end());

		for (auto p : objsOrdering) {
			bool objIsUnsaved = unsavedObjs.find(p.second) != unsavedObjs.end();
			addLevelPackObjectToListView(p.second, p.first, objIsUnsaved, objIsUnsaved ? unsavedObjs[p.second]->getName() : "");
		}
	}
}

std::map<int, std::shared_ptr<LevelPackObject>>& AttacksListView::getUnsavedLevelPackObjects() {
	return mainEditorWindow.getUnsavedAttacks();
}

void AttacksListView::updateLevelPackObjectInLevelPack(std::shared_ptr<LevelPackObject> obj) {
	mainEditorWindow.updateAttack(std::dynamic_pointer_cast<EditorAttack>(obj));
}

void AttacksListView::deleteLevelPackObjectInLevelPack(int id) {
	mainEditorWindow.deleteAttack(id);
	reloadLevelPackObjectTabInMainEditorWindow(id);
}

std::shared_ptr<LevelPackObject> AttacksListView::getLevelPackObjectFromLevelPack(int id) {
	return levelPack->getAttack(id);
}

std::set<int> AttacksListView::getNextLevelPackObjectIDs(int count) {
	return levelPack->getNextAttackIDs(count);
}

void AttacksListView::openLevelPackObjectInMainEditorWindow(int id) {
	mainEditorWindow.openLeftPanelAttack(id);
}

void AttacksListView::reloadLevelPackObjectTabInMainEditorWindow(int id) {
	mainEditorWindow.reloadAttackTab(id);
}

void AttacksListView::saveLevelPackObjects(std::set<size_t> ids) {
	mainEditorWindow.saveAttackChanges(ids);
}

std::string AttacksListView::getPasteIntoConfirmationPrompt() {
	return "Overwrite the selected attack(s) with the copied attack(s)? This will reload their tabs if they are currently open.";
}

std::string AttacksListView::getDeleteLevelPackObjectsInUseConfirmationPrompt() {
	return "Are you sure you want to delete the selected attack(s)? One or more are currently being used by an attack pattern.";
}

bool AttacksListView::getLevelPackObjectIsInUse(int id) {
	return levelPack->getAttackUsers(id).size() != 0;
}

std::string AttacksListView::getLevelPackObjectDisplayName() {
	return "attack";
}

void AttacksListView::onPasteIntoConfirmation(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, std::vector<std::shared_ptr<LevelPackObject>> newObjects) {
	if (choice == EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::YES) {
		std::vector<std::shared_ptr<EditorAttack>> derivedObjects;
		for (auto obj : newObjects) {
			derivedObjects.push_back(std::dynamic_pointer_cast<EditorAttack>(obj));
		}
		mainEditorWindow.overwriteAttacks(derivedObjects, &undoStack);
		reload();
	}
}

void LevelPackObjectsListView::onDeleteConfirmation(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, 
	std::vector<std::pair<std::shared_ptr<LevelPackObject>, bool>> deletedObjects) {

	if (choice == EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::YES) {
		deleteObjects(getListView()->getSelectedItemIndices(), deletedObjects);
	}
}

void LevelPackObjectsListView::addLevelPackObjectToListView(int objID, std::string objName, bool objIsUnsaved, std::string objUnsavedName) {
	// If the object is unsaved, signal it with an asterisk
	if (objIsUnsaved) {
		listView->addItem(format(UNSAVED_LEVEL_PACK_OBJECT_ITEM_FORMAT, objID, objUnsavedName.c_str()));
	} else {
		listView->addItem(format(SAVED_LEVEL_PACK_OBJECT_ITEM_FORMAT, objID, objName.c_str()));
	}
	int index = listView->getItemCount() - 1;
	levelPackObjectIDToListViewIndexMap[objID] = index;
	listViewIndexToLevelPackObjectIDMap[index] = objID;
}

void LevelPackObjectsListView::deleteObjects(std::set<size_t> selectedIndices, std::vector<std::pair<std::shared_ptr<LevelPackObject>, bool>> deletedObjs) {
	undoStack.execute(UndoableCommand([this, selectedIndices]() {
		for (int index : selectedIndices) {
			int id = getLevelPackObjectIDFromIndex(index);
			deleteLevelPackObjectInLevelPack(id);
		}

		reload();
	}, [this, deletedObjs]() {
		auto& unsavedObjs = getUnsavedLevelPackObjects();

		for (std::pair<std::shared_ptr<LevelPackObject>, bool> pair : deletedObjs) {
			updateLevelPackObjectInLevelPack(pair.first);
			// The object was originally unsaved, so add it back to unsavedObjs
			if (pair.second) {
				unsavedObjs[pair.first->getID()] = pair.first;
			}
		}

		reload();

		// Select the objects added back in
		std::set<size_t> newSelectedIndices;
		for (std::pair<std::shared_ptr<LevelPackObject>, bool> pair : deletedObjs) {
			newSelectedIndices.insert(getIndexFromLevelPackObjectID(pair.first->getID()));
		}
		getListView()->setSelectedItems(newSelectedIndices);
	}));
}

AttackPatternsListView::AttackPatternsListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize) : LevelPackObjectsListView("EditorAttackPattern", mainEditorWindow, clipboard, undoStackSize) {
}

void AttackPatternsListView::addLevelPackObjectsToListView() {
	auto& unsavedObjs = getUnsavedLevelPackObjects();
	if (sortOption == SORT_OPTION::ID) {
		for (auto it = levelPack->getAttackPatternIteratorBegin(); it != levelPack->getAttackPatternIteratorEnd(); it++) {
			if (it->first < 0) {
				continue;
			}

			bool objIsUnsaved = unsavedObjs.find(it->first) != unsavedObjs.end();
			addLevelPackObjectToListView(it->first, it->second->getName(), objIsUnsaved, objIsUnsaved ? unsavedObjs[it->first]->getName() : "");
		}
	} else {
		std::vector<std::pair<std::string, int>> objsOrdering;
		for (auto it = levelPack->getAttackPatternIteratorBegin(); it != levelPack->getAttackPatternIteratorEnd(); it++) {
			if (it->first < 0) {
				continue;
			}

			if (unsavedObjs.find(it->first) != unsavedObjs.end()) {
				objsOrdering.push_back(std::make_pair(unsavedObjs[it->first]->getName(), it->first));
			} else {
				objsOrdering.push_back(std::make_pair(it->second->getName(), it->first));
			}
		}
		std::sort(objsOrdering.begin(), objsOrdering.end());

		for (auto p : objsOrdering) {
			bool objIsUnsaved = unsavedObjs.find(p.second) != unsavedObjs.end();
			addLevelPackObjectToListView(p.second, p.first, objIsUnsaved, objIsUnsaved ? unsavedObjs[p.second]->getName() : "");
		}
	}
}

std::map<int, std::shared_ptr<LevelPackObject>>& AttackPatternsListView::getUnsavedLevelPackObjects() {
	return mainEditorWindow.getUnsavedAttackPatterns();
}

void AttackPatternsListView::updateLevelPackObjectInLevelPack(std::shared_ptr<LevelPackObject> obj) {
	mainEditorWindow.updateAttackPattern(std::dynamic_pointer_cast<EditorAttackPattern>(obj));
}

void AttackPatternsListView::deleteLevelPackObjectInLevelPack(int id) {
	mainEditorWindow.deleteAttackPattern(id);
	reloadLevelPackObjectTabInMainEditorWindow(id);
}

std::shared_ptr<LevelPackObject> AttackPatternsListView::getLevelPackObjectFromLevelPack(int id) {
	return levelPack->getAttackPattern(id);
}

std::set<int> AttackPatternsListView::getNextLevelPackObjectIDs(int count) {
	return levelPack->getNextAttackPatternIDs(count);
}

void AttackPatternsListView::openLevelPackObjectInMainEditorWindow(int id) {
	mainEditorWindow.openLeftPanelAttackPattern(id);
}

void AttackPatternsListView::reloadLevelPackObjectTabInMainEditorWindow(int id) {
	mainEditorWindow.reloadAttackPatternTab(id);
}

void AttackPatternsListView::saveLevelPackObjects(std::set<size_t> ids) {
	mainEditorWindow.saveAttackPatternChanges(ids);
}

std::string AttackPatternsListView::getPasteIntoConfirmationPrompt() {
	return "Overwrite the selected attack patterns(s) with the copied attack patterns(s)? This will reload their tabs if they are currently open.";
}

std::string AttackPatternsListView::getDeleteLevelPackObjectsInUseConfirmationPrompt() {
	return "Are you sure you want to delete the selected attack patterns(s)? One or more are currently being used by an enemy phase or by the player.";
}

bool AttackPatternsListView::getLevelPackObjectIsInUse(int id) {
	return levelPack->getAttackPatternEnemyUsers(id).size() != 0 || levelPack->getAttackPatternIsUsedByPlayer(id);
}

std::string AttackPatternsListView::getLevelPackObjectDisplayName() {
	return "attack pattern";
}

void AttackPatternsListView::onPasteIntoConfirmation(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, std::vector<std::shared_ptr<LevelPackObject>> newObjects) {
	if (choice == EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::YES) {
		std::vector<std::shared_ptr<EditorAttackPattern>> derivedObjects;
		for (auto obj : newObjects) {
			derivedObjects.push_back(std::dynamic_pointer_cast<EditorAttackPattern>(obj));
		}
		mainEditorWindow.overwriteAttackPatterns(derivedObjects, &undoStack);
		reload();
	}
}