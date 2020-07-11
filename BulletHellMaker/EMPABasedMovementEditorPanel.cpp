#include "EMPABasedMovementEditorPanel.h"
#include "EditorMovablePointActionPanel.h"

const std::string EMPABasedMovementEditorPanel::EMPA_TAB_NAME_FORMAT = "Action %d";
const int EMPABasedMovementEditorPanel::EMPA_TAB_NAME_FORMAT_NUMBER_INDEX = 7;

EditorMovablePointActionsListView::EditorMovablePointActionsListView(EMPABasedMovementEditorPanel& empaBasedMovementEditorPanel, Clipboard& clipboard, int undoStackSize) : 
	CopyPasteable("EMPAction"), empaBasedMovementEditorPanel(empaBasedMovementEditorPanel), clipboard(clipboard), undoStack(UndoStack(undoStackSize)) {
	getListView()->setMultiSelect(true);
}

std::pair<std::shared_ptr<CopiedObject>, std::string> EditorMovablePointActionsListView::copyFrom() {
	return empaBasedMovementEditorPanel.manualCopy();
}

std::string EditorMovablePointActionsListView::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	return empaBasedMovementEditorPanel.manualPaste(pastedObject);
}

std::string EditorMovablePointActionsListView::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	return empaBasedMovementEditorPanel.manualPaste2(pastedObject);
}

bool EditorMovablePointActionsListView::handleEvent(sf::Event event) {
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
			empaBasedMovementEditorPanel.manualDelete();
			return true;
		}
	}
	return false;
}

UndoStack& EditorMovablePointActionsListView::getUndoStack() {
	return undoStack;
}

void EditorMovablePointActionsListView::manualCopy() {
	clipboard.copy(this);
}

void EditorMovablePointActionsListView::manualPaste() {
	clipboard.paste(this);
}

void EditorMovablePointActionsListView::manualPaste2() {
	clipboard.paste2(this);
}

EditorMovablePointActionsListPanel::EditorMovablePointActionsListPanel(EditorWindow& parentWindow, EMPABasedMovementEditorPanel& empaBasedMovementEditorPanel, Clipboard& clipboard) : 
	parentWindow(parentWindow), empaBasedMovementEditorPanel(empaBasedMovementEditorPanel), clipboard(clipboard) {
	empasListView = EditorMovablePointActionsListView::create(empaBasedMovementEditorPanel, clipboard);
	add(empasListView);

	// Catching the signals from empaAddButton and empaDeleteButton are done in EMPABasedMovementEditorPanel because it needs
	// to modify its tabs when EMPAs are added/deleted.

	// Add button
	empaAddButton = tgui::Button::create();
	empaAddButton->setText("+");
	empaAddButton->setPosition(0, 0);
	empaAddButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	add(empaAddButton);

	// Delete button
	empaDeleteButton = tgui::Button::create();
	empaDeleteButton->setText("-");
	empaDeleteButton->setPosition(tgui::bindRight(empaAddButton), tgui::bindTop(empaAddButton));
	empaDeleteButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	add(empaDeleteButton);

	// List view
	empasListView->setPosition(0, tgui::bindBottom(empaAddButton));
	// Right click menu
	// Menu for single attack selection
	auto rightClickMenuPopupSingleSelection = createMenuPopup({
		std::make_pair("Open", [this]() {
			this->empaBasedMovementEditorPanel.manualOpenEMPA(empasListView->getListView()->getSelectedItemIndex());
		}),
		std::make_pair("Copy", [this]() {
			empasListView->manualCopy();
		}),
		std::make_pair("Paste", [this]() {
			empasListView->manualPaste();
		}),
		std::make_pair("Paste (override this)", [this]() {
			empasListView->manualPaste2();
		}),
		std::make_pair("Delete", [this]() {
			this->empaBasedMovementEditorPanel.manualDelete();
		})
		});
	// Menu for multiple attack selections
	auto rightClickMenuPopupMultiSelection = createMenuPopup({
		std::make_pair("Copy", [this]() {
			empasListView->manualCopy();
		}),
		std::make_pair("Paste", [this]() {
			empasListView->manualPaste();
		}),
		std::make_pair("Paste (override these)", [this]() {
			empasListView->manualPaste2();
		}),
		std::make_pair("Delete", [this]() {
			this->empaBasedMovementEditorPanel.manualDelete();
		})
		});
	empasListView->getListView()->connect("RightClicked", [this, rightClickMenuPopupSingleSelection, rightClickMenuPopupMultiSelection](int index) {
		std::set<std::size_t> selectedItemIndices = empasListView->getListView()->getSelectedItemIndices();
		if (selectedItemIndices.find(index) != selectedItemIndices.end()) {
			// Right clicked a selected item

			// Open the corresponding menu
			if (selectedItemIndices.size() == 1) {
				this->parentWindow.addPopupWidget(rightClickMenuPopupSingleSelection, this->parentWindow.getMousePos().x, this->parentWindow.getMousePos().y, 150, rightClickMenuPopupSingleSelection->getSize().y);
			} else {
				this->parentWindow.addPopupWidget(rightClickMenuPopupMultiSelection, this->parentWindow.getMousePos().x, this->parentWindow.getMousePos().y, 150, rightClickMenuPopupMultiSelection->getSize().y);
			}
		} else {
			// Right clicked a nonselected item

			// Select the right clicked item
			empasListView->getListView()->setSelectedItem(index);

			// Open the menu normally
			this->parentWindow.addPopupWidget(rightClickMenuPopupSingleSelection, this->parentWindow.getMousePos().x, this->parentWindow.getMousePos().y, 150, rightClickMenuPopupSingleSelection->getSize().y);
		}
	});

	connect("SizeChanged", [this](sf::Vector2f newSize) {
		empasListView->setSize(newSize.x, newSize.y - tgui::bindBottom(empaAddButton));
	});
}

bool EditorMovablePointActionsListPanel::handleEvent(sf::Event event) {
	return empasListView->handleEvent(event);
}

std::shared_ptr<EditorMovablePointActionsListView> EditorMovablePointActionsListPanel::getEmpasListView() {
	return empasListView;
}

std::shared_ptr<tgui::Button> EditorMovablePointActionsListPanel::getEMPAAddButton() {
	return empaAddButton;
}

std::shared_ptr<tgui::Button> EditorMovablePointActionsListPanel::getEMPADeleteButton() {
	return empaDeleteButton;
}

EMPABasedMovementEditorPanel::EMPABasedMovementEditorPanel(EditorWindow& parentWindow, Clipboard& clipboard) : parentWindow(parentWindow), clipboard(clipboard) {
	tabs = TabsWithPanel::create(parentWindow);
	tabs->setSize("100%", "100%");

	visualizer = EMPAsVisualizerPanel::create(parentWindow, *this, clipboard);
	tabs->addTab("Visualizer", visualizer, true, false);

	visualizer->getEmpasListView()->getListView()->connect("DoubleClicked", [this](int index) {
		manualOpenEMPA(index);
	});
	visualizer->getEmpasListPanel()->getEMPAAddButton()->connect("Pressed", [this]() {
		int newEMPAIndex;
		std::set<size_t> curSelectedIndices = visualizer->getEmpasListView()->getListView()->getSelectedItemIndices();
		if (curSelectedIndices.size() == 0) {
			newEMPAIndex = actions.size();
		} else {
			newEMPAIndex = *curSelectedIndices.begin();
		}

		visualizer->getEmpasListView()->getUndoStack().execute(UndoableCommand([this, newEMPAIndex]() {
			actions.insert(actions.begin() + newEMPAIndex, std::make_shared<StayStillAtLastPositionEMPA>(0));

			// Rename all tabs for higher or equal EMPA indices
			std::vector<int> actionIndicesToBeRenamed;
			// Start at 1 because the first tab is the visualizer
			std::vector<std::string> tabNames = tabs->getTabNames();
			for (int i = 1; i < tabNames.size(); i++) {
				int actionIndex = std::stoi(tabNames[i].substr(EMPA_TAB_NAME_FORMAT_NUMBER_INDEX));
				if (actionIndex >= newEMPAIndex) {
					actionIndicesToBeRenamed.push_back(actionIndex);
				}
			}
			// Since action indicies need to be incremented, iterate starting at the highest action indices first so that
			// there can't be a situation where 2 tabs have the same name
			std::sort(actionIndicesToBeRenamed.begin(), actionIndicesToBeRenamed.end(), std::greater<int>());
			for (int actionIndex : actionIndicesToBeRenamed) {
				tabs->renameTab(format(EMPA_TAB_NAME_FORMAT, actionIndex), format(EMPA_TAB_NAME_FORMAT, actionIndex + 1));
			}

			std::set<size_t> newSelectedIndices;
			newSelectedIndices.insert(newEMPAIndex);
			updateEMPAList(true, newSelectedIndices);
			onEMPAListModify.emit(this, cloneActions(), sumOfDurations);
		}, [this, newEMPAIndex]() {
			actions.erase(actions.begin() + newEMPAIndex);

			// Close the tab if it is open
			std::string tabName = format(EMPA_TAB_NAME_FORMAT, newEMPAIndex);
			if (tabs->hasTab(tabName)) {
				tabs->removeTab(tabName);
			}
			
			// Rename all tabs for higher EMPA indices
			std::vector<int> actionIndicesToBeRenamed;
			// Start at 1 because the first tab is the visualizer
			std::vector<std::string> tabNames = tabs->getTabNames();
			for (int i = 1; i < tabNames.size(); i++) {
				int actionIndex = std::stoi(tabNames[i].substr(EMPA_TAB_NAME_FORMAT_NUMBER_INDEX));
				if (actionIndex > newEMPAIndex) {
					actionIndicesToBeRenamed.push_back(actionIndex);
				}
			}
			// Since action indicies need to be decremented, iterate starting at the lowest action indices first so that
			// there can't be a situation where 2 tabs have the same name
			std::sort(actionIndicesToBeRenamed.begin(), actionIndicesToBeRenamed.end());
			for (int actionIndex : actionIndicesToBeRenamed) {
				tabs->renameTab(format(EMPA_TAB_NAME_FORMAT, actionIndex), format(EMPA_TAB_NAME_FORMAT, actionIndex - 1));
			}

			updateEMPAList();
			onEMPAListModify.emit(this, cloneActions(), sumOfDurations);
		}));
	});
	visualizer->getEmpasListPanel()->getEMPADeleteButton()->connect("Pressed", [this]() {
		manualDelete();
	});

	add(tabs);
}

tgui::Signal& EMPABasedMovementEditorPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onEMPAListModify.getName())) {
		return onEMPAListModify;
	}
	return tgui::Panel::getSignal(signalName);
}

bool EMPABasedMovementEditorPanel::handleEvent(sf::Event event) {
	return tabs->handleEvent(event);
}

float EMPABasedMovementEditorPanel::getSumOfDurations() {
	return sumOfDurations;
}

void EMPABasedMovementEditorPanel::setActions(std::vector<std::shared_ptr<EMPAction>> actions) {
	this->actions.clear();
	for (auto action : actions) {
		this->actions.push_back(std::dynamic_pointer_cast<EMPAction>(action->clone()));
	}
	// sumOfDurations calculation already done in updateEMPAList() so no need to do it here
	updateEMPAList();
}

void EMPABasedMovementEditorPanel::manualDelete() {
	std::set<size_t> curSelectedIndices = visualizer->getEmpasListView()->getListView()->getSelectedItemIndices();
	if (curSelectedIndices.size() == 0) {
		return;
	}

	std::vector<std::shared_ptr<EMPAction>> oldEMPAs;
	for (int index : curSelectedIndices) {
		oldEMPAs.push_back(actions[index]);
	}
	visualizer->getEmpasListView()->getUndoStack().execute(UndoableCommand([this, curSelectedIndices]() {
		for (auto it = curSelectedIndices.rbegin(); it != curSelectedIndices.rend(); it++) {
			int index = *it;
			// curSelectedIndices is a set so we can remove elements using erase() if done in decreasing index order
			actions.erase(actions.begin() + index);

			// Close the tab if it is open
			std::string tabName = format(EMPA_TAB_NAME_FORMAT, index);
			if (tabs->hasTab(tabName)) {
				tabs->removeTab(tabName);
			}
		}

		// Maps EMPA index to be renamed to the negative change in index as a result of this deletion
		std::map<int, int> actionIndicesToBeRenamed;
		std::vector<std::string> tabNames = tabs->getTabNames();
		// Sort so that tabNames is increasing in EMPA index
		std::vector<int> tValues;
		// Skip the first tab because it's the visualizer
		for (int i = 1; i < tabNames.size(); i++) {
			tValues.push_back(std::stoi(tabNames[i].substr(EMPA_TAB_NAME_FORMAT_NUMBER_INDEX)));
		}
		std::sort(tValues.begin(), tValues.end());
		std::vector<int> deletedAsVector;
		std::copy(curSelectedIndices.begin(), curSelectedIndices.end(), std::back_inserter(deletedAsVector));
		// O(|tabs| + |curSelectedIndices|) algorithm to calculate all necessary values of actionIndicesToBeRenamed
		int tPtr = 0, dPtr = 0, numSeen = 1;
		while (tPtr < tValues.size() && dPtr < curSelectedIndices.size()) {
			int t = tValues[tPtr];
			int d = deletedAsVector[dPtr];
			if (t < d) {
				tPtr++;
			} else {
				actionIndicesToBeRenamed[t] = numSeen;
				numSeen++;
				dPtr++;
			}
		}
		if (tPtr < tValues.size()) {
			// assign the rest

			for (int i = 1; i < tabNames.size(); i++) {
				int t = std::stoi(tabNames[i].substr(EMPA_TAB_NAME_FORMAT_NUMBER_INDEX));
				if (t > deletedAsVector[deletedAsVector.size() - 1]) {
					actionIndicesToBeRenamed[t] = numSeen - 1;
				}
			}
		}

		// Do the actual renaming.
		// Since action indicies need to be decremented, iterate starting at the lowest action indices first so that
		// there can't be a situation where 2 tabs have the same name. Maps are already sorted.
		for (auto it = actionIndicesToBeRenamed.begin(); it != actionIndicesToBeRenamed.end(); it++) {
			int actionIndex = it->first;
			tabs->renameTab(format(EMPA_TAB_NAME_FORMAT, actionIndex), format(EMPA_TAB_NAME_FORMAT, actionIndex - it->second));
		}

		// Update and unselect
		updateEMPAList(true, std::set<size_t>());
		onEMPAListModify.emit(this, cloneActions(), sumOfDurations);
	}, [this, curSelectedIndices, oldEMPAs]() {
		int i = 0;
		for (auto it = curSelectedIndices.begin(); it != curSelectedIndices.end(); it++) {
			int index = *it;
			actions.insert(actions.begin() + index, oldEMPAs[i]);
			i++;
		}

		// Maps EMPA index to be renamed to the negative change in index as a result of this deletion
		std::map<int, int> actionIndicesToBeRenamed;
		std::vector<std::string> tabNames = tabs->getTabNames();
		// Sort so that tabNames is increasing in EMPA index
		std::vector<int> tValues;
		// Skip the first tab because it's the visualizer
		for (int i = 1; i < tabNames.size(); i++) {
			tValues.push_back(std::stoi(tabNames[i].substr(EMPA_TAB_NAME_FORMAT_NUMBER_INDEX)));
		}
		std::sort(tValues.begin(), tValues.end());
		std::vector<int> deletedAsVector;
		std::copy(curSelectedIndices.begin(), curSelectedIndices.end(), std::back_inserter(deletedAsVector));
		// O(|tabs| + |curSelectedIndices|) algorithm to calculate all necessary values of actionIndicesToBeRenamed
		int tPtr = 0, dPtr = 0, numSeen = 0;
		while (tPtr < tValues.size() && dPtr < curSelectedIndices.size()) {
			int t = tValues[tPtr];
			int d = deletedAsVector[dPtr];
			if (t + numSeen < d) {
				if (numSeen != 0 && actionIndicesToBeRenamed.count(t) == 0) {
					actionIndicesToBeRenamed[t] = numSeen;
				}
				tPtr++;
			} else {
				if (actionIndicesToBeRenamed.count(t) == 0) {
					numSeen++;
					actionIndicesToBeRenamed[t] = numSeen;
				}
				if (t >= d) {
					dPtr++;
				} else {
					tPtr++;
				}
			}
		}
		if (tPtr < tValues.size()) {
			// assign the rest

			for (int i = 1; i < tabNames.size(); i++) {
				int t = std::stoi(tabNames[i].substr(EMPA_TAB_NAME_FORMAT_NUMBER_INDEX));
				if (t >= deletedAsVector[deletedAsVector.size() - 1]) {
					actionIndicesToBeRenamed[t] = numSeen - 1;
				}
			}
		}

		// Do the actual renaming.
		// Since action indicies need to be incremented, iterate starting at the largest action indices first so that
		// there can't be a situation where 2 tabs have the same name. Maps are already sorted.
		for (auto it = actionIndicesToBeRenamed.rbegin(); it != actionIndicesToBeRenamed.rend(); it++) {
			int actionIndex = it->first;
			tabs->renameTab(format(EMPA_TAB_NAME_FORMAT, actionIndex), format(EMPA_TAB_NAME_FORMAT, actionIndex + it->second));
		}

		updateEMPAList(true, curSelectedIndices);
		onEMPAListModify.emit(this, cloneActions(), sumOfDurations);
	}));
}

std::pair<std::shared_ptr<CopiedObject>, std::string> EMPABasedMovementEditorPanel::manualCopy() {
	auto selected = visualizer->getEmpasListView()->getListView()->getSelectedItemIndices();
	if (selected.size() > 0) {
		std::vector<std::shared_ptr<EMPAction>> copied;
		for (int index : selected) {
			// No need to clone here because CopiedEMPActions's constructor will clone it
			copied.push_back(actions[index]);
		}
		std::string copyMessage;
		if (selected.size() == 1) {
			copyMessage = "Copied 1 movable point action";
		} else {
			copyMessage = "Copied " + std::to_string(selected.size()) + " movable point actions";
		}
		return std::make_pair(std::make_shared<CopiedEMPActions>(visualizer->getEmpasListView()->getID(), copied), copyMessage);
	}
	return std::make_pair(nullptr, "");
}

std::string EMPABasedMovementEditorPanel::manualPaste(std::shared_ptr<CopiedObject> pastedObject) {
	// Can just modify the code for manualDelete() a bit

	auto derived = std::static_pointer_cast<CopiedEMPActions>(pastedObject);
	if (derived) {
		std::set<size_t> curSelectedIndices = visualizer->getEmpasListView()->getListView()->getSelectedItemIndices();
		int pasteAtIndex;
		if (curSelectedIndices.size() == 0) {
			pasteAtIndex = actions.size();
		} else {
			pasteAtIndex = *curSelectedIndices.begin();
		}

		int numPasted = derived->getActionsCount();
		visualizer->getEmpasListView()->getUndoStack().execute(UndoableCommand([this, derived, pasteAtIndex, numPasted]() {
			auto newEMPAs = derived->getActions();
			std::set<size_t> pastedIndices;
			for (int i = pasteAtIndex; i < pasteAtIndex + numPasted; i++) {
				pastedIndices.insert(i);
			}

			for (int i = pasteAtIndex; i < pasteAtIndex + numPasted; i++) {
				actions.insert(actions.begin() + i, newEMPAs[i - pasteAtIndex]);
			}

			// Rename every tab of higher or equal index.
			// Start at 1 because the first tab is visualizer
			std::vector<std::string> tabNames = tabs->getTabNames();
			for (int i = 1; i < tabNames.size(); i++) {
				int empaIndex = std::stoi(tabNames[i].substr(EMPA_TAB_NAME_FORMAT_NUMBER_INDEX));
				tabs->renameTab(tabNames[i], format(EMPA_TAB_NAME_FORMAT, empaIndex + numPasted));
			}

			updateEMPAList(true, pastedIndices);
			onEMPAListModify.emit(this, cloneActions(), sumOfDurations);
		}, [this, pasteAtIndex, numPasted]() {
			std::set<size_t> pastedIndices;
			for (int i = pasteAtIndex; i < pasteAtIndex + numPasted; i++) {
				pastedIndices.insert(i);
			}

			for (auto it = pastedIndices.rbegin(); it != pastedIndices.rend(); it++) {
				int index = *it;
				// curSelectedIndices is a set so we can remove elements using erase() if done in decreasing index order
				actions.erase(actions.begin() + index);

				// Close the tab if it is open
				std::string tabName = format(EMPA_TAB_NAME_FORMAT, index);
				if (tabs->hasTab(tabName)) {
					tabs->removeTab(tabName);
				}
			}

			// Maps EMPA index to be renamed to the negative change in index as a result of this deletion
			std::map<int, int> actionIndicesToBeRenamed;
			std::vector<std::string> tabNames = tabs->getTabNames();
			// Sort so that tabNames is increasing in EMPA index
			std::vector<int> tValues;
			// Skip the first tab because it's the visualizer
			for (int i = 1; i < tabNames.size(); i++) {
				tValues.push_back(std::stoi(tabNames[i].substr(EMPA_TAB_NAME_FORMAT_NUMBER_INDEX)));
			}
			std::sort(tValues.begin(), tValues.end());
			std::vector<int> deletedAsVector;
			std::copy(pastedIndices.begin(), pastedIndices.end(), std::back_inserter(deletedAsVector));
			// O(|tabs| + |curSelectedIndices|) algorithm to calculate all necessary values of actionIndicesToBeRenamed
			int tPtr = 0, dPtr = 0, numSeen = 1;
			while (tPtr < tValues.size() && dPtr < pastedIndices.size()) {
				int t = tValues[tPtr];
				int d = deletedAsVector[dPtr];
				if (t < d) {
					tPtr++;
				} else {
					actionIndicesToBeRenamed[t] = numSeen;
					numSeen++;
					dPtr++;
				}
			}
			if (tPtr < tValues.size()) {
				// assign the rest

				for (int i = 1; i < tabNames.size(); i++) {
					int t = std::stoi(tabNames[i].substr(EMPA_TAB_NAME_FORMAT_NUMBER_INDEX));
					if (t > deletedAsVector[deletedAsVector.size() - 1]) {
						actionIndicesToBeRenamed[t] = numSeen - 1;
					}
				}
			}

			// Do the actual renaming.
			// Since action indicies need to be decremented, iterate starting at the lowest action indices first so that
			// there can't be a situation where 2 tabs have the same name. Maps are already sorted.
			for (auto it = actionIndicesToBeRenamed.begin(); it != actionIndicesToBeRenamed.end(); it++) {
				int actionIndex = it->first;
				tabs->renameTab(format(EMPA_TAB_NAME_FORMAT, actionIndex), format(EMPA_TAB_NAME_FORMAT, actionIndex - it->second));
			}

			updateEMPAList();
			onEMPAListModify.emit(this, cloneActions(), sumOfDurations);
		}));

		if (numPasted == 1) {
			return "Pasted 1 movable point action";
		} else {
			return "Pasted " + std::to_string(numPasted) + " movable point actions";
		}
	}
	return "";
}

std::string EMPABasedMovementEditorPanel::manualPaste2(std::shared_ptr<CopiedObject> pastedObject) {
	std::set<size_t> selectedIndices = visualizer->getEmpasListView()->getListView()->getSelectedItemIndices();
	auto derived = std::static_pointer_cast<CopiedEMPActions>(pastedObject);
	if (selectedIndices.size() > 0 && derived) {
		std::vector<std::shared_ptr<EMPAction>> copiedEMPAs = derived->getActions();
		if (copiedEMPAs.size() == selectedIndices.size()) {
			// See "Why paste2 in LevelPackObjectsListView can't be undoable" in personal notes for explanation on why this can't be undoable
			parentWindow.promptConfirmation("Overwrite the selected action(s) with the copied action(s)? This will reload their tabs if they are currently open.", copiedEMPAs)->sink()
				.connect<EMPABasedMovementEditorPanel, &EMPABasedMovementEditorPanel::onPasteIntoConfirmation>(this);
		} else {
			std::string s1;
			if (copiedEMPAs.size() == 1) {
				s1 = "1 movable point action was";
			} else {
				s1 = std::to_string(copiedEMPAs.size()) + " movable point actions were";
			}
			std::string s2;
			if (selectedIndices.size() == 1) {
				s2 = "only 1 is";
			} else {
				s2 = std::to_string(selectedIndices.size()) + " are";
			}
			return "Size mismatch. " + s1 + " copied but " + s2 + " selected";
		}
	}
	return "";
}

void EMPABasedMovementEditorPanel::manualOpenEMPA(int index) {
	std::string tabName = format(EMPA_TAB_NAME_FORMAT, index);

	if (tabs->hasTab(tabName)) {
		tabs->selectTab(tabName);
	} else {
		std::shared_ptr<tgui::Panel> empaPanel = createEMPAPanel(actions[index], index, visualizer->getEmpasListView());
		tabs->addTab(tabName, empaPanel, true, true);
	}
}

void EMPABasedMovementEditorPanel::propagateChangesToChildren() {
	auto tabNames = tabs->getTabNames();
	// Start at the index where EMPA tabs start
	for (int i = 1; i < tabNames.size(); i++) {
		std::dynamic_pointer_cast<EditorMovablePointActionPanel>(tabs->getTab(tabNames[i]))->updateSymbolTables(symbolTables);
	}
}

ValueSymbolTable EMPABasedMovementEditorPanel::getLevelPackObjectSymbolTable() {
	// Unused
	return ValueSymbolTable();
}

std::shared_ptr<tgui::Panel> EMPABasedMovementEditorPanel::createEMPAPanel(std::shared_ptr<EMPAction> empa, int index, std::shared_ptr<ListViewScrollablePanel> empiActions) {
	std::shared_ptr<EditorMovablePointActionPanel> empaPanel = EditorMovablePointActionPanel::create(parentWindow, clipboard, std::dynamic_pointer_cast<EMPAction>(empa->clone()));
	empaPanel->updateSymbolTables(symbolTables);
	empaPanel->connect("EMPAModified", [this, index, empiActions](std::shared_ptr<EMPAction> value) {
		this->actions[index] = value;

		updateEMPAList();
		onEMPAListModify.emit(this, cloneActions(), sumOfDurations);
	});
	return empaPanel;
}

void EMPABasedMovementEditorPanel::updateEMPAList(bool setSelectedIndices, std::set<size_t> newSelectedIndices) {
	std::shared_ptr<ListViewScrollablePanel> actionsListBoxScrollablePanel = visualizer->getEmpasListView();
	auto listView = actionsListBoxScrollablePanel->getListView();

	auto oldSelectedIndices = listView->getSelectedItemIndices();
	listView->removeAllItems();
	sumOfDurations = 0;
	for (int i = 0; i < actions.size(); i++) {
		listView->addItem(actions[i]->getGuiFormat() + " (d=" + formatNum(actions[i]->getTime()) + "; t=" + formatNum(sumOfDurations) + " to t=" + formatNum(sumOfDurations + actions[i]->getTime()) + ")");
		sumOfDurations += actions[i]->getTime();
	}
	actionsListBoxScrollablePanel->onListViewItemsUpdate();
	if (setSelectedIndices) {
		listView->setSelectedItems(newSelectedIndices);
	} else {
		// Reselect previous EMPAs
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

			listView->setSelectedItems(afterRemoval);
		}
	}
}

std::vector<std::shared_ptr<EMPAction>> EMPABasedMovementEditorPanel::cloneActions() {
	std::vector<std::shared_ptr<EMPAction>> copy;
	for (auto action : actions) {
		copy.push_back(std::dynamic_pointer_cast<EMPAction>(action->clone()));
	}
	return copy;
}

void EMPABasedMovementEditorPanel::reloadEMPATab(int empaIndex) {
	std::string tabName = format(EMPA_TAB_NAME_FORMAT, empaIndex);
	// Do nothing if the tab doesn't exist
	if (tabs->hasTab(tabName)) {
		// Remove the tab and then re-insert it at its old index
		bool tabWasSelected = tabs->getSelectedTab() == tabName;
		int tabIndex = tabs->getTabIndex(tabName);
		tabs->removeTab(tabName);

		// Create the tab
		std::shared_ptr<tgui::Panel> empaPanel = createEMPAPanel(actions[empaIndex], empaIndex, visualizer->getEmpasListView());
		tabs->insertTab(tabName, empaPanel, tabIndex, tabWasSelected, true);
	}
}

void EMPABasedMovementEditorPanel::onPasteIntoConfirmation(bool confirmed, std::vector<std::shared_ptr<EMPAction>> newEMPAs) {
	if (confirmed) {
		std::set<size_t> curSelectedIndices = visualizer->getEmpasListView()->getListView()->getSelectedItemIndices();
		assert(curSelectedIndices.size() == newEMPAs.size());

		std::vector<std::shared_ptr<EMPAction>> oldEMPAs;
		for (auto it = curSelectedIndices.begin(); it != curSelectedIndices.end(); it++) {
			oldEMPAs.push_back(actions[*it]);
		}

		visualizer->getEmpasListView()->getUndoStack().execute(UndoableCommand([this, curSelectedIndices, newEMPAs]() {
			int i = 0;
			for (auto it = curSelectedIndices.begin(); it != curSelectedIndices.end(); it++) {
				actions[*it] = std::dynamic_pointer_cast<EMPAction>(newEMPAs[i]->clone());
				i++;

				// Reload tab
				reloadEMPATab(*it);
			}
			
			updateEMPAList();
			onEMPAListModify.emit(this, cloneActions(), sumOfDurations);
		}, [this, curSelectedIndices, oldEMPAs]() {
			int i = 0;
			for (auto it = curSelectedIndices.begin(); it != curSelectedIndices.end(); it++) {
				actions[*it] = std::dynamic_pointer_cast<EMPAction>(oldEMPAs[i]->clone());
				i++;

				// Reload tab
				reloadEMPATab(*it);
			}

			updateEMPAList();
			onEMPAListModify.emit(this, cloneActions(), sumOfDurations);
		}));
	}
}

EMPAsVisualizerPanel::EMPAsVisualizerPanel(EditorWindow& parentWindow, EMPABasedMovementEditorPanel& empaBasedMovementEditorPanel, Clipboard& clipboard) {
	empasListPanel = EditorMovablePointActionsListPanel::create(parentWindow, empaBasedMovementEditorPanel, clipboard);
	mainPanel = tgui::Panel::create();

	// TODO: add visualizer thing to mainPanel

	empasListPanel->setSize("25%", "100%");
	mainPanel->setSize("75%", "100%");
	empasListPanel->setPosition(0, 0);
	mainPanel->setPosition(tgui::bindRight(empasListPanel), tgui::bindTop(empasListPanel));

	add(empasListPanel);
	add(mainPanel);
}

bool EMPAsVisualizerPanel::handleEvent(sf::Event event) {
	// mainPanel doesn't need to handle any events
	return empasListPanel->handleEvent(event);
}

std::shared_ptr<EditorMovablePointActionsListView> EMPAsVisualizerPanel::getEmpasListView() {
	return empasListPanel->getEmpasListView();
}

std::shared_ptr<EditorMovablePointActionsListPanel> EMPAsVisualizerPanel::getEmpasListPanel() {
	return empasListPanel;
}
