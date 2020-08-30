#include <Editor/LevelPackObjectUseRelationship/LevelPackObjectUseRelationshipEditor.h>


LevelPackObjectUseRelationshipEditor::LevelPackObjectUseRelationshipEditor(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, 
	std::string copyPasteableID, bool enableMultipleRelationships, int undoStackSize)
		: CopyPasteable(copyPasteableID), mainEditorWindow(mainEditorWindow), clipboard(clipboard), undoStackSize(undoStackSize) {
	relationshipEditorPanel = tgui::Panel::create();
	relationshipEditorPanel->setVisible(false);
	add(relationshipEditorPanel);

	if (enableMultipleRelationships) {
		relationshipEditorPanel->setSize("30%", "100%");
	} else {
		relationshipEditorPanel->setSize("100%", "100%");
	}
}

LevelPackObjectUseRelationshipEditor::~LevelPackObjectUseRelationshipEditor() {

}

bool LevelPackObjectUseRelationshipEditor::handleEvent(sf::Event event) {
	if (relationshipsListView && relationshipsListView->isFocused() && relationshipsListView->handleEvent(event)) {
		return true;
	} else if (event.type == sf::Event::KeyPressed) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::Z) {
				manualRelationshipEditorPanelUndo();
				return true;
			} else if (event.key.code == sf::Keyboard::Y) {
				manualRelationshipEditorPanelRedo();
				return true;
			} else if (event.key.code == sf::Keyboard::V) {
				clipboard.paste2(this);
				return true;
			}
		}
	}
	return false;
}

std::pair<std::shared_ptr<CopiedObject>, std::string> LevelPackObjectUseRelationshipEditor::copyFrom() {
	// Can't copy from this widget
	return std::make_pair(nullptr, "");
}

std::string LevelPackObjectUseRelationshipEditor::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	// Default to paste2
	return paste2Into(pastedObject);
}

void LevelPackObjectUseRelationshipEditor::deleteSelectedListItems() {
	if (relationshipsListView) {
		std::set<size_t> curSelectedIndices = relationshipsListView->getListView()->getSelectedItemIndices();
		if (curSelectedIndices.size() == 0) {
			return;
		}

		std::vector<std::pair<std::shared_ptr<LevelPackObjectUseRelationship>, UndoStack>> oldRelationships;
		for (int index : curSelectedIndices) {
			oldRelationships.push_back(relationships[index]);
		}

		this->getCurrentlySelectedRelationshipUndoStack()->execute(UndoableCommand([this, curSelectedIndices]() {
			for (auto it = curSelectedIndices.rbegin(); it != curSelectedIndices.rend(); it++) {
				int index = *it;
				// curSelectedIndices is a set so we can remove elements using erase() if done in decreasing index order
				relationships.erase(relationships.begin() + index);
			}

			relationshipsListView->repopulateRelationships(getRelationships());

			if (curSelectedIndices.find(relationshipEditorPanelCurrentRelationshipIndex) != curSelectedIndices.end()) {
				// If the relationship being edited by relationshipEditorPanel is currently selected, hide relationshipEditorPanel first
				relationshipEditorPanel->setVisible(false);
			}
		}, [this, curSelectedIndices, oldRelationships]() {
			// Re-insert deleted relationships
			int i = 0;
			for (auto it = curSelectedIndices.begin(); it != curSelectedIndices.end(); it++) {
				int index = *it;
				relationships.insert(relationships.begin() + index, oldRelationships[i]);
				i++;
			}

			relationshipsListView->repopulateRelationships(getRelationships());
		}));
	}
}

void LevelPackObjectUseRelationshipEditor::insertRelationships(int insertAt, std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> insertedRelationships) {
	int numInserted = insertedRelationships.size();
	for (int i = insertAt; i < insertAt + numInserted; i++) {
		relationships.insert(relationships.begin() + i, std::make_pair(insertedRelationships[i - insertAt], UndoStack(undoStackSize)));
	}
}

void LevelPackObjectUseRelationshipEditor::onRelationshipEditorPanelRelationshipModify(std::shared_ptr<LevelPackObjectUseRelationship> newRelationship) {
	if (relationshipEditorPanelCurrentRelationshipIndex == -1) {
		return;
	}

	relationships[relationshipEditorPanelCurrentRelationshipIndex].first = newRelationship;
	onRelationshipsChangeHelper();
}

void LevelPackObjectUseRelationshipEditor::setupRelationshipListView() {
	relationshipsListViewPanel = tgui::Panel::create();
	relationshipsListViewPanel->setSize("30%", "100%");
	add(relationshipsListViewPanel);

	relationshipEditorPanel->setSize("70%", "100%");
	relationshipEditorPanel->setPosition(tgui::bindRight(relationshipsListViewPanel), tgui::bindTop(relationshipsListViewPanel));

	// Add button
	auto addButton = tgui::Button::create();
	addButton->setText("+");
	addButton->setPosition(0, 0);
	addButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	addButton->connect("Pressed", [this]() {
		// TODO
	});
	relationshipsListViewPanel->add(addButton);

	// List view
	instantiateRelationshipListView(mainEditorWindow, clipboard);
	relationshipsListView->connect("ListModified", [this]() {
		onRelationshipsChangeHelper();
	});
	relationshipsListView->setPosition(0, tgui::bindBottom(addButton));
	relationshipsListView->setSize("100%", tgui::bindHeight(relationshipsListViewPanel) - tgui::bindBottom(addButton));
	{
		// Right click menu
		// Menu for single selection
		auto rightClickMenuPopupSingleSelection = createMenuPopup({
			std::make_pair("Open", [this]() {
				int index = relationshipsListView->getListView()->getSelectedItemIndex();
				if (index == -1) {
					relationshipEditorPanel->setVisible(false);
				} else {
					openRelationshipEditorPanelIndex(index);
				}
			}),
			std::make_pair("Copy", [this]() {
				relationshipsListView->manualCopy();
			}),
			std::make_pair("Paste", [this]() {
				relationshipsListView->manualPaste();
			}),
			std::make_pair("Paste (override this)", [this]() {
				relationshipsListView->manualPaste2();
			}),
			std::make_pair("Delete", [this]() {
				relationshipsListView->manualDelete();
			})
		});
		// Menu for multiple selections
		auto rightClickMenuPopupMultiSelection = createMenuPopup({
			std::make_pair("Copy", [this]() {
				relationshipsListView->manualCopy();
			}),
			std::make_pair("Paste", [this]() {
				relationshipsListView->manualPaste();
			}),
			std::make_pair("Paste (override these)", [this]() {
				relationshipsListView->manualPaste2();
			}),
			std::make_pair("Delete", [this]() {
				relationshipsListView->manualDelete();
			})
		});
		relationshipsListView->getListView()->connect("RightClicked", [this, rightClickMenuPopupSingleSelection, rightClickMenuPopupMultiSelection](int index) {
			std::set<std::size_t> selectedItemIndices = relationshipsListView->getListView()->getSelectedItemIndices();
			auto mousePos = this->mainEditorWindow.getMousePos();
			if (selectedItemIndices.find(index) != selectedItemIndices.end()) {
				// Right clicked a selected item

				// Open the corresponding menu
				if (selectedItemIndices.size() == 1) {
					this->mainEditorWindow.addPopupWidget(rightClickMenuPopupSingleSelection, mousePos.x, mousePos.y, 150, rightClickMenuPopupSingleSelection->getSize().y);
				} else {
					this->mainEditorWindow.addPopupWidget(rightClickMenuPopupMultiSelection, mousePos.x, mousePos.y, 150, rightClickMenuPopupMultiSelection->getSize().y);
				}
			} else {
				// Right clicked a nonselected item

				// Select the right clicked item
				relationshipsListView->getListView()->setSelectedItem(index);

				// Open the menu normally
				this->mainEditorWindow.addPopupWidget(rightClickMenuPopupSingleSelection, mousePos.x, mousePos.y, 150, rightClickMenuPopupSingleSelection->getSize().y);
			}
		});
	}
	relationshipsListView->getListView()->connect("DoubleClicked", [this](int index) {
		if (index == -1) {
			return;
		}

		openRelationshipEditorPanelIndex(index);
	});
	relationshipsListViewPanel->add(relationshipsListView);
}

UndoStack* LevelPackObjectUseRelationshipEditor::getCurrentlySelectedRelationshipUndoStack() {
	if (relationshipEditorPanelCurrentRelationshipIndex != -1) {
		return &relationships[relationshipEditorPanelCurrentRelationshipIndex].second;
	}
	return nullptr;
}

std::shared_ptr<LevelPackObjectUseRelationship> LevelPackObjectUseRelationshipEditor::getCurrentlySelectedRelationship() {
	if (relationshipEditorPanelCurrentRelationshipIndex != -1) {
		return relationships[relationshipEditorPanelCurrentRelationshipIndex].first;
	}
	return nullptr;
}

void LevelPackObjectUseRelationshipEditor::deleteRelationships(std::set<size_t> indices) {
	for (auto it = indices.rbegin(); it != indices.rend(); it++) {
		int index = *it;
		// We can use erase() if done in decreasing index order
		relationships.erase(relationships.begin() + index);

		// Hide relationshipEditorPanel if it's currently editing a removed relationship
		if (relationshipEditorPanelCurrentRelationshipIndex == index) {
			relationshipEditorPanel->setVisible(false);
		}
	}
}

void LevelPackObjectUseRelationshipEditor::replaceRelationships(std::set<size_t> indices, std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> replacements) {
	int i = 0;
	for (auto it = indices.begin(); it != indices.end(); it++) {
		relationships[*it] = std::make_pair(replacements[i], UndoStack(undoStackSize));
		i++;

		if (relationshipEditorPanelCurrentRelationshipIndex == *it) {
			initializeRelationshipEditorPanelWidgetsData(replacements[i], *it);
		}
	}
}

std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> LevelPackObjectUseRelationshipEditor::getRelationshipsSubset(std::set<size_t> indices) {
	std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> subset;
	for (size_t i : indices) {
		subset.push_back(relationships[i].first);
	}
	return subset;
}

std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> LevelPackObjectUseRelationshipEditor::getRelationships() {
	std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> ret;
	for (auto p : relationships) {
		ret.push_back(p.first);
	}
	return ret;
}

int LevelPackObjectUseRelationshipEditor::getRelationshipsCount() {
	return relationships.size();
}

void LevelPackObjectUseRelationshipEditor::openRelationshipEditorPanelIndex(int index) {
	this->relationshipEditorPanelCurrentRelationshipIndex = index;
	relationshipEditorPanel->setVisible(true);
	initializeRelationshipEditorPanelWidgetsData(relationships[index].first, index);
}

void LevelPackObjectUseRelationshipEditor::manualRelationshipEditorPanelUndo() {
	if (relationshipEditorPanelCurrentRelationshipIndex != -1) {
		relationships[relationshipEditorPanelCurrentRelationshipIndex].second.undo();
	}
}

void LevelPackObjectUseRelationshipEditor::manualRelationshipEditorPanelRedo() {
	if (relationshipEditorPanelCurrentRelationshipIndex != -1) {
		relationships[relationshipEditorPanelCurrentRelationshipIndex].second.redo();
	}
}

void LevelPackObjectUseRelationshipEditor::onRelationshipsChangeHelper() {
	std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> prunedRelationships;
	for (std::pair<std::shared_ptr<LevelPackObjectUseRelationship>, UndoStack> p : relationships) {
		prunedRelationships.push_back(p.first);
	}
	onRelationshipsChange(prunedRelationships);
}

LevelPackObjectUseRelationshipListView::LevelPackObjectUseRelationshipListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard,
	LevelPackObjectUseRelationshipEditor& parentRelationshipEditor, std::string copyPasteableID, int undoStackSize)
	: ListViewScrollablePanel(), CopyPasteable(copyPasteableID), mainEditorWindow(mainEditorWindow), clipboard(clipboard),
	parentRelationshipEditor(parentRelationshipEditor), undoStack(UndoStack(undoStackSize)) {
}

LevelPackObjectUseRelationshipListView::~LevelPackObjectUseRelationshipListView() {
}

bool LevelPackObjectUseRelationshipListView::handleEvent(sf::Event event) {
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
			}
		}
		if (event.key.code == sf::Keyboard::Delete) {
			manualDelete();
			return true;
		}
	}
	return false;
}

void LevelPackObjectUseRelationshipListView::manualCopy() {
	clipboard.copy(this);
}

void LevelPackObjectUseRelationshipListView::manualPaste() {
	clipboard.paste(this);
	repopulateRelationships(parentRelationshipEditor.getRelationships());
}

void LevelPackObjectUseRelationshipListView::manualPaste2() {
	clipboard.paste2(this);
	repopulateRelationships(parentRelationshipEditor.getRelationships());
}

void LevelPackObjectUseRelationshipListView::manualDelete() {
	parentRelationshipEditor.deleteSelectedListItems();
}

void LevelPackObjectUseRelationshipListView::manualSelectAll() {
	int count = getListView()->getItemCount();
	std::set<size_t> selected;
	for (int i = 0; i < count; i++) {
		selected.insert(i);
	}
	getListView()->setSelectedItems(selected);
}

void LevelPackObjectUseRelationshipListView::repopulateRelationships(std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> relationships) {
	std::shared_ptr<tgui::ListView> listView = getListView();
	std::set<size_t> oldSelectedIndices = listView->getSelectedItemIndices();

	listView->removeAllItems();

	for (std::shared_ptr<LevelPackObjectUseRelationship> relationship : relationships) {
		listView->addItem(getRelationshipListViewText(relationship));
	}

	// Reselect previous indices
	if (oldSelectedIndices.size() > 0) {
		// Remove indices that no longer exist
		std::set<size_t> afterRemoval;
		for (auto it = oldSelectedIndices.begin(); it != oldSelectedIndices.end(); it++) {
			if (*it >= listView->getItemCount()) {
				break;
			} else {
				afterRemoval.insert(*it);
			}
		}

		if (afterRemoval.size() == 1) {
			listView->setSelectedItem(*afterRemoval.begin());
		} else {
			listView->setSelectedItems(afterRemoval);
		}
	}
}

tgui::Signal& LevelPackObjectUseRelationshipListView::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onListModify.getName())) {
		return onListModify;
	}
	return ListViewScrollablePanel::getSignal(signalName);
}

UndoStack& LevelPackObjectUseRelationshipListView::getUndoStack() {
	return undoStack;
}