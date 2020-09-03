#include <Editor/LevelPackObjectUseRelationship/AttackPatternToAttackUseRelationshipEditor.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/Util/EditorUtils.h>

std::vector<std::tuple<std::string, int, ExprSymbolTable>> AttackPatternToAttackUseRelationship::convertRelationshipVectorToDataVector(
	std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> relationships) {

	std::vector<std::tuple<std::string, int, ExprSymbolTable>> data;
	for (std::shared_ptr<LevelPackObjectUseRelationship> relationship : relationships) {
		data.push_back(std::static_pointer_cast<AttackPatternToAttackUseRelationship>(relationship)->getData());
	}
	return data;
}

std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> AttackPatternToAttackUseRelationship::convertDataVectorToRelationshipVector(
	std::vector<std::tuple<std::string, int, ExprSymbolTable>> data) {

	std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> relationships;
	for (std::tuple<std::string, int, ExprSymbolTable> dataPoint : data) {
		relationships.push_back(std::make_shared<AttackPatternToAttackUseRelationship>(dataPoint));
	}
	return relationships;
}

AttackPatternToAttackUseRelationshipListView::AttackPatternToAttackUseRelationshipListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, 
	UndoStack& undoStack, LevelPackObjectUseRelationshipEditor& parentRelationshipEditor) 
	: LevelPackObjectUseRelationshipListView(mainEditorWindow, clipboard, undoStack, parentRelationshipEditor, ATTACK_PATTERN_TO_ATTACK_USE_RELATIONSHIP_COPY_PASTE_ID) {
}

CopyOperationResult AttackPatternToAttackUseRelationshipListView::copyFrom() {
	auto selectedIndices = getListView()->getSelectedItemIndices();
	if (selectedIndices.size() > 0) {
		return CopyOperationResult(std::make_shared<CopiedAttackPatternToAttackUseRelationship>(getID(),
			AttackPatternToAttackUseRelationship::convertRelationshipVectorToDataVector(parentRelationshipEditor.getRelationshipsSubset(selectedIndices))),
			"Copied " + std::to_string(selectedIndices.size()) + " attack pattern to attack relationships");
	}
	return CopyOperationResult(nullptr, "");
}

PasteOperationResult AttackPatternToAttackUseRelationshipListView::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	auto derived = std::static_pointer_cast<CopiedAttackPatternToAttackUseRelationship>(pastedObject);
	if (derived) {
		std::set<size_t> curSelectedIndices = getListView()->getSelectedItemIndices();
		int pasteAtIndex;
		if (curSelectedIndices.size() == 0) {
			pasteAtIndex = parentRelationshipEditor.getRelationshipsCount();
		} else {
			pasteAtIndex = *curSelectedIndices.begin();
		}

		int numPasted = derived->getRelationshipsCount();
		undoStack.execute(UndoableCommand([this, derived, pasteAtIndex, numPasted]() {
			std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> newRelationships = AttackPatternToAttackUseRelationship::convertDataVectorToRelationshipVector(derived->getRelationships());

			std::set<size_t> pastedIndices;
			for (int i = pasteAtIndex; i < pasteAtIndex + numPasted; i++) {
				pastedIndices.insert(i);
			}

			parentRelationshipEditor.insertRelationships(pasteAtIndex, newRelationships);
			// manualPaste() already takes care of list view repopulation

			onListModify.emit(this);
		}, [this, pasteAtIndex, numPasted]() {
			std::set<size_t> pastedIndices;
			for (int i = pasteAtIndex; i < pasteAtIndex + numPasted; i++) {
				pastedIndices.insert(i);
			}

			parentRelationshipEditor.deleteRelationships(pastedIndices);
			// manualPaste() already takes care of list view repopulation

			onListModify.emit(this);
		}));

		if (numPasted == 1) {
			return PasteOperationResult(true, "Pasted 1 attack pattern to attack relationship");
		} else {
			return PasteOperationResult(true, "Pasted " + std::to_string(numPasted) + " attack pattern to attack relationships");
		}
	}
	return PasteOperationResult(false, "Type mismatch");
}

PasteOperationResult AttackPatternToAttackUseRelationshipListView::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	std::set<size_t> selectedIndices = getListView()->getSelectedItemIndices();
	auto derived = std::static_pointer_cast<CopiedAttackPatternToAttackUseRelationship>(pastedObject);
	if (selectedIndices.size() > 0 && derived) {
		std::vector<std::tuple<std::string, int, ExprSymbolTable>> copiedRelationships = derived->getRelationships();
		if (copiedRelationships.size() == selectedIndices.size()) {
			// See "Why paste2 in LevelPackObjectsListView can't be undoable" in personal notes for explanation on why this can't be undoable (not the same thing but similar logic)
			mainEditorWindow.promptConfirmation("Overwrite the selected relationship(s) with the copied relationship(s)? This will reload the relationship editor if it is editing a selected relationship.", 
				copiedRelationships)->sink().connect<AttackPatternToAttackUseRelationshipListView, &AttackPatternToAttackUseRelationshipListView::onPasteIntoConfirmation>(this);
			return PasteOperationResult(true, "");
		} else {
			std::string s1;
			if (copiedRelationships.size() == 1) {
				s1 = "1 attack pattern to attack relationship was";
			} else {
				s1 = std::to_string(copiedRelationships.size()) + " attack pattern to attack relationships were";
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

std::string AttackPatternToAttackUseRelationshipListView::getRelationshipListViewText(std::shared_ptr<LevelPackObjectUseRelationship> relationship) {
	std::shared_ptr<AttackPatternToAttackUseRelationship> derived = std::static_pointer_cast<AttackPatternToAttackUseRelationship>(relationship);
	std::tuple<std::string, int, ExprSymbolTable> data = derived->getData();
	return format("[time=\"%s\"] [Attack ID=%d]", std::get<0>(data).c_str(), std::get<1>(data));
}

void AttackPatternToAttackUseRelationshipListView::onPasteIntoConfirmation(bool confirmed, std::vector<std::tuple<std::string, int, ExprSymbolTable>> newRelationshipsData) {
	if (confirmed) {
		std::set<size_t> curSelectedIndices = getListView()->getSelectedItemIndices();
		std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> oldRelationships = parentRelationshipEditor.getRelationshipsSubset(curSelectedIndices);

		std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> newRelationships;
		for (auto t : newRelationshipsData) {
			newRelationships.push_back(std::make_shared<AttackPatternToAttackUseRelationship>(t));
		}

		undoStack.execute(UndoableCommand([this, curSelectedIndices, newRelationships]() {
			parentRelationshipEditor.replaceRelationships(curSelectedIndices, newRelationships);

			onListModify.emit(this);
		}, [this, curSelectedIndices, oldRelationships]() {
			parentRelationshipEditor.replaceRelationships(curSelectedIndices, oldRelationships);

			onListModify.emit(this);
		}));
	}
}

AttackPatternToAttackUseRelationshipEditor::AttackPatternToAttackUseRelationshipEditor(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, UndoStack& undoStack,
	std::vector<std::tuple<std::string, int, ExprSymbolTable>> initialRelationshipsData)
	: LevelPackObjectUseRelationshipEditor(mainEditorWindow, clipboard, undoStack, ATTACK_PATTERN_TO_ATTACK_USE_RELATIONSHIP_COPY_PASTE_ID, true) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	timeLabel = tgui::Label::create();
	timeLabel->setTextSize(TEXT_SIZE);
	timeLabel->setText("Execution begin time");
	timeLabel->setToolTip(createToolTip("The time in seconds at which the attack is executed. \
Any variables used in here must be defined by the attack pattern, not in the variables list associated with this relationship."));
	relationshipEditorPanel->add(timeLabel);

	timeEditBox = EditBox::create();
	timeEditBox->setTextSize(TEXT_SIZE);
	timeEditBox->connect("ValueChanged", [this](std::string value) {
		if (ignoreSignals) {
			return;
		}

		std::shared_ptr<AttackPatternToAttackUseRelationship> currentRelationship = std::static_pointer_cast<AttackPatternToAttackUseRelationship>(getCurrentlySelectedRelationship());
		std::string oldValue = std::get<0>(currentRelationship->getData());
		if (value == oldValue) {
			return;
		}
		int currentlyOpenIndex = this->relationshipEditorPanelCurrentRelationshipIndex;
		this->undoStack.execute(UndoableCommand(
			[this, currentRelationship, value, currentlyOpenIndex]() {
			currentRelationship->setData(std::make_tuple(value, std::get<1>(currentRelationship->getData()), std::get<2>(currentRelationship->getData())));

			bool prevIgnoreSignals = ignoreSignals;
			ignoreSignals = true;
			openRelationshipEditorPanelIndex(currentlyOpenIndex);
			relationshipsListView->repopulateRelationships(getRelationships());
			ignoreSignals = prevIgnoreSignals;

			onRelationshipEditorPanelRelationshipModify(currentRelationship);
		}, 
			[this, currentRelationship, oldValue, currentlyOpenIndex]() {
			currentRelationship->setData(std::make_tuple(oldValue, std::get<1>(currentRelationship->getData()), std::get<2>(currentRelationship->getData())));

			bool prevIgnoreSignals = ignoreSignals;
			ignoreSignals = true;
			openRelationshipEditorPanelIndex(currentlyOpenIndex);
			relationshipsListView->repopulateRelationships(getRelationships());
			ignoreSignals = prevIgnoreSignals;

			onRelationshipEditorPanelRelationshipModify(currentRelationship);
		}));
	});
	relationshipEditorPanel->add(timeEditBox);

	idLabel = tgui::Label::create();
	idLabel->setTextSize(TEXT_SIZE);
	idLabel->setText("Attack ID");
	idLabel->setToolTip(createToolTip("The ID of the attack to be executed. Note that this references attacks by ID only so that if the \
attack with the same ID is deleted/modified, this attack pattern will not know about the change. This can lead to references to non-existing attacks \
(if the attack with the ID is deleted) or unintentionally different attacks (if the attack with the ID is deleted and a new one with the same ID is created, or \
if it is modified with the intention of changing only one attack pattern's behavior while it is actually being used by others as well)."));
	relationshipEditorPanel->add(idLabel);

	idEditBox = NumericalEditBoxWithLimits::create();
	idEditBox->setIntegerMode(true);
	idEditBox->setMin(0);
	idEditBox->setTextSize(TEXT_SIZE);
	idEditBox->connect("ValueChanged", [this](float value) {
		if (ignoreSignals) {
			return;
		}
		// Take care of potential rounding errors
		int id = std::lrint(value);

		std::shared_ptr<AttackPatternToAttackUseRelationship> currentRelationship = std::static_pointer_cast<AttackPatternToAttackUseRelationship>(getCurrentlySelectedRelationship());
		int oldValue = std::get<1>(currentRelationship->getData());
		if (value == oldValue) {
			return;
		}
		int currentlyOpenIndex = this->relationshipEditorPanelCurrentRelationshipIndex;
		this->undoStack.execute(UndoableCommand(
			[this, currentRelationship, value, currentlyOpenIndex]() {
			currentRelationship->setData(std::make_tuple(std::get<0>(currentRelationship->getData()), value, std::get<2>(currentRelationship->getData())));

			bool prevIgnoreSignals = ignoreSignals;
			ignoreSignals = true;
			openRelationshipEditorPanelIndex(currentlyOpenIndex);
			relationshipsListView->repopulateRelationships(getRelationships());
			ignoreSignals = prevIgnoreSignals;

			onRelationshipEditorPanelRelationshipModify(currentRelationship);
		},
			[this, currentRelationship, oldValue, currentlyOpenIndex]() {
			currentRelationship->setData(std::make_tuple(std::get<0>(currentRelationship->getData()), oldValue, std::get<2>(currentRelationship->getData())));

			bool prevIgnoreSignals = ignoreSignals;
			ignoreSignals = true;
			openRelationshipEditorPanelIndex(currentlyOpenIndex);
			relationshipsListView->repopulateRelationships(getRelationships());
			ignoreSignals = prevIgnoreSignals;

			onRelationshipEditorPanelRelationshipModify(currentRelationship);
		}));
	});
	relationshipEditorPanel->add(idEditBox);

	symbolTableEditorLabel = tgui::Label::create();
	symbolTableEditorLabel->setTextSize(TEXT_SIZE);
	symbolTableEditorLabel->setText("Variables to pass to referenced object");
	relationshipEditorPanel->add(symbolTableEditorLabel);

	symbolTableEditor = ExprSymbolTableEditor::create(undoStack);
	symbolTableEditor->connect("ValueChanged", [this](ExprSymbolTable value) {
		if (ignoreSignals) {
			return;
		}

		// No need for an undoable command here because individual changes to components in ExprSymbolTableEditor are
		// added to the currently opened relationship's undo stack
		std::shared_ptr<AttackPatternToAttackUseRelationship> currentRelationship = std::static_pointer_cast<AttackPatternToAttackUseRelationship>(getCurrentlySelectedRelationship());
		std::tuple<std::string, int, ExprSymbolTable> currentRelationshipData = currentRelationship->getData();
		currentRelationship->setData(std::make_tuple(std::get<0>(currentRelationshipData), std::get<1>(currentRelationshipData), value));

		relationshipsListView->repopulateRelationships(getRelationships());
	});
	relationshipEditorPanel->add(symbolTableEditor);

	timeLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	timeEditBox->setPosition(tgui::bindLeft(timeLabel), tgui::bindBottom(timeLabel) + GUI_LABEL_PADDING_Y);
	idLabel->setPosition(tgui::bindLeft(timeLabel), tgui::bindBottom(timeEditBox) + GUI_PADDING_Y);
	idEditBox->setPosition(tgui::bindLeft(timeLabel), tgui::bindBottom(idLabel) + GUI_LABEL_PADDING_Y);
	symbolTableEditorLabel->setPosition(tgui::bindLeft(timeLabel), tgui::bindBottom(idEditBox) + GUI_PADDING_Y);
	symbolTableEditor->setPosition(tgui::bindLeft(timeLabel), tgui::bindBottom(symbolTableEditorLabel) + GUI_LABEL_PADDING_Y);

	relationshipEditorPanel->connect("SizeChanged", [this](sf::Vector2f newSize) {
		timeEditBox->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
		idEditBox->setSize(newSize.x - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
		symbolTableEditor->setSize(newSize.x - GUI_PADDING_X * 2, newSize.y - (symbolTableEditorLabel->getPosition().y + symbolTableEditorLabel->getSize().y) - GUI_PADDING_Y);
	});

	setupRelationshipListView();
	relationshipsListView->repopulateRelationships(AttackPatternToAttackUseRelationship::convertDataVectorToRelationshipVector(initialRelationshipsData));
	insertRelationships(0, AttackPatternToAttackUseRelationship::convertDataVectorToRelationshipVector(initialRelationshipsData));
}

PasteOperationResult AttackPatternToAttackUseRelationshipEditor::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	auto derived = std::static_pointer_cast<CopiedAttackPatternToAttackUseRelationship>(pastedObject);
	if (derived) {
		std::vector<std::tuple<std::string, int, ExprSymbolTable>> copiedRelationships = derived->getRelationships();
		if (copiedRelationships.size() == 1) {
			// See "Why paste2 in LevelPackObjectsListView can't be undoable" in personal notes for explanation on why this can't be undoable (not the same thing but similar logic)
			mainEditorWindow.promptConfirmation("Overwrite the currently open relationship with the copied relationship?",
				copiedRelationships)->sink().connect<AttackPatternToAttackUseRelationshipEditor, &AttackPatternToAttackUseRelationshipEditor::onPasteIntoConfirmation>(this);
			return PasteOperationResult(true, "");
		} else {
			return PasteOperationResult(false, "Cannot overwrite this relationship's properties when copying more than one attack pattern to attack relationship.");
		}
	}
	return PasteOperationResult(false, "Type mismatch");
}

tgui::Signal& AttackPatternToAttackUseRelationshipEditor::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onRelationshipsModify.getName())) {
		return onRelationshipsModify;
	}
	return LevelPackObjectUseRelationshipEditor::getSignal(signalName);
}

void AttackPatternToAttackUseRelationshipEditor::setSymbolTablesHierarchy(std::vector<ValueSymbolTable> symbolTablesHierarchy) {
	symbolTableEditor->setSymbolTablesHierarchy(symbolTablesHierarchy);
}

void AttackPatternToAttackUseRelationshipEditor::initializeRelationshipEditorPanelWidgetsData(std::shared_ptr<LevelPackObjectUseRelationship> relationship, int relationshipIndex) {
	std::shared_ptr<AttackPatternToAttackUseRelationship> derived = std::static_pointer_cast<AttackPatternToAttackUseRelationship>(relationship);
	std::tuple<std::string, int, ExprSymbolTable> data = derived->getData();

	bool prevIgnoreSignals = ignoreSignals;
	ignoreSignals = true;
	timeEditBox->setText(std::get<0>(data));
	idEditBox->setValue(std::get<1>(data));
	symbolTableEditor->setExprSymbolTable(std::get<2>(data));
	// No need to set symbolTableEditor's ValueSymbolTable hierarchy here because the same hierarchy will be used for all relationships
	ignoreSignals = prevIgnoreSignals;
}

void AttackPatternToAttackUseRelationshipEditor::onRelationshipsChange(std::vector<std::shared_ptr<LevelPackObjectUseRelationship>> newRelationships) {
	std::vector<std::tuple<std::string, int, ExprSymbolTable>> data = AttackPatternToAttackUseRelationship::convertRelationshipVectorToDataVector(newRelationships);
	onRelationshipsModify.emit(this, data);
}

void AttackPatternToAttackUseRelationshipEditor::instantiateRelationshipListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard) {
	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	relationshipsListView = AttackPatternToAttackUseRelationshipListView::create(mainEditorWindow, clipboard, undoStack, *this);
}

std::shared_ptr<LevelPackObjectUseRelationship> AttackPatternToAttackUseRelationshipEditor::instantiateDefaultRelationship() {
	return std::make_shared<AttackPatternToAttackUseRelationship>(std::make_tuple("", 0, ExprSymbolTable()));
}

void AttackPatternToAttackUseRelationshipEditor::onPasteIntoConfirmation(bool confirmed, std::vector<std::tuple<std::string, int, ExprSymbolTable>> newRelationshipData) {
	if (confirmed || newRelationshipData.size() != 1) {
		std::shared_ptr<AttackPatternToAttackUseRelationship> currentRelationship = std::static_pointer_cast<AttackPatternToAttackUseRelationship>(getCurrentlySelectedRelationship());
		std::tuple<std::string, int, ExprSymbolTable> oldRelationshipData = currentRelationship->getData();
		undoStack.execute(UndoableCommand(
			[this, currentRelationship, newRelationshipData]() {
			currentRelationship->setData(newRelationshipData[0]);

			bool prevIgnoreSignals = ignoreSignals;
			ignoreSignals = true;
			timeEditBox->setText(std::get<0>(newRelationshipData[0]));
			idEditBox->setValue(std::get<1>(newRelationshipData[0]));
			symbolTableEditor->setExprSymbolTable(std::get<2>(newRelationshipData[0]));
			relationshipsListView->repopulateRelationships(getRelationships());
			ignoreSignals = prevIgnoreSignals;

			onRelationshipEditorPanelRelationshipModify(currentRelationship);
		},
			[this, currentRelationship, oldRelationshipData]() {
			currentRelationship->setData(oldRelationshipData);

			bool prevIgnoreSignals = ignoreSignals;
			ignoreSignals = true;
			timeEditBox->setText(std::get<0>(oldRelationshipData));
			idEditBox->setValue(std::get<1>(oldRelationshipData));
			symbolTableEditor->setExprSymbolTable(std::get<2>(oldRelationshipData));
			relationshipsListView->repopulateRelationships(getRelationships());
			ignoreSignals = prevIgnoreSignals;

			onRelationshipEditorPanelRelationshipModify(currentRelationship);
		}));
	}
}

AttackPatternToAttackUseRelationship::AttackPatternToAttackUseRelationship(std::tuple<std::string, int, ExprSymbolTable> data)
	: data(data) {
}

void AttackPatternToAttackUseRelationship::setData(std::tuple<std::string, int, ExprSymbolTable> data) {
	this->data = data;
}

std::tuple<std::string, int, ExprSymbolTable> AttackPatternToAttackUseRelationship::getData() {
	return data;
}
