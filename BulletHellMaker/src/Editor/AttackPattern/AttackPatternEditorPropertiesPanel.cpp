#include <Editor/AttackPattern/AttackPatternEditorPropertiesPanel.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/EditorWindow.h>

AttackPatternEditorPropertiesPanel::AttackPatternEditorPropertiesPanel(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::shared_ptr<EditorAttackPattern> attackPattern, int undoStackSize)
	: CopyPasteable("EditorAttackPattern"), mainEditorWindow(mainEditorWindow), clipboard(clipboard), attackPattern(attackPattern), undoStack(UndoStack(undoStackSize)) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	std::shared_ptr<tgui::Label> id = tgui::Label::create();
	id->setTextSize(TEXT_SIZE);
	id->setText("Attack pattern ID " + std::to_string(attackPattern->getID()));
	add(id);

	std::shared_ptr<tgui::Label> nameLabel = tgui::Label::create();
	nameLabel->setTextSize(TEXT_SIZE);
	nameLabel->setText("Name");
	add(nameLabel);

	name = EditBox::create();
	name->setTextSize(TEXT_SIZE);
	name->setText(attackPattern->getName());
	add(name);

	std::shared_ptr<tgui::Label> relationshipEditorLabel = tgui::Label::create();
	relationshipEditorLabel->setTextSize(TEXT_SIZE);
	relationshipEditorLabel->setText("Attacks");
	add(relationshipEditorLabel);

	relationshipEditor = AttackPatternToAttackUseRelationshipEditor::create(mainEditorWindow, clipboard, undoStack, attackPattern->getAttacks());
	relationshipEditor->connect("RelationshipsModified", [this](std::vector<std::tuple<std::string, int, ExprSymbolTable>> newRelationships) {
		this->attackPattern->setAttacks(newRelationships);
		onAttackPatternModify.emit(this);
	});
	add(relationshipEditor);

	std::shared_ptr<tgui::Label> usedByLabel = tgui::Label::create();
	usedByLabel->setTextSize(TEXT_SIZE);
	usedByLabel->setText("Used by");
	add(usedByLabel);

	usedBy = ListViewScrollablePanel::create();
	usedBy->getListView()->setTextSize(TEXT_SIZE);
	add(usedBy);

	id->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	nameLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(id) + GUI_PADDING_Y);
	name->setPosition(tgui::bindLeft(id), tgui::bindBottom(nameLabel) + GUI_LABEL_PADDING_Y);
	relationshipEditorLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(name) + GUI_PADDING_Y);
	relationshipEditor->setPosition(tgui::bindLeft(id), tgui::bindBottom(relationshipEditorLabel) + GUI_LABEL_PADDING_Y);
	usedByLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(relationshipEditor) + GUI_PADDING_Y);
	usedBy->setPosition(tgui::bindLeft(id), tgui::bindBottom(usedByLabel) + GUI_LABEL_PADDING_Y);

	connect("SizeChanged", [this](sf::Vector2f newSize) {
		name->setSize(newSize.x - GUI_PADDING_X * 2, tgui::bindHeight(name));
		relationshipEditor->setSize(newSize.x - GUI_PADDING_X * 2, "50%");
		usedBy->setSize(newSize.x - GUI_PADDING_X * 2, newSize.y - tgui::bindTop(usedBy) - GUI_PADDING_Y);
	});

	name->connect("ValueChanged", [this](std::string text) {
		if (ignoreSignals) {
			return;
		}

		std::string oldName = this->attackPattern->getName();

		if (text != oldName) {
			undoStack.execute(UndoableCommand([this, text]() {
				this->attackPattern->setName(text);
				name->setText(text);
				onAttackPatternModify.emit(this);
			}, [this, oldName]() {
				this->attackPattern->setName(oldName);
				name->setText(oldName);
				onAttackPatternModify.emit(this);
			}));
		}
	});
}

std::pair<std::shared_ptr<CopiedObject>, std::string> AttackPatternEditorPropertiesPanel::copyFrom() {
	// Can't copy from this widget
	return std::make_pair(nullptr, "");
}

std::string AttackPatternEditorPropertiesPanel::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	// Same thing as paste2Into
	return paste2Into(pastedObject);
}

std::string AttackPatternEditorPropertiesPanel::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	// Paste the first copied EditorAttack to override attack's properties
	auto derived = std::static_pointer_cast<CopiedLevelPackObject>(pastedObject);
	if (derived) {
		if (derived->getLevelPackObjectsCount() == 1) {
			std::shared_ptr<EditorAttackPattern> attackPattern = std::dynamic_pointer_cast<EditorAttackPattern>(derived->getLevelPackObjects()[0]);
			std::string newName = attackPattern->getName();
			std::vector<std::tuple<std::string, int, ExprSymbolTable>> newAttacks = attackPattern->getAttacks();
			CopiedAttackPatternProperties newProperties(newName, newAttacks);
			mainEditorWindow.promptConfirmation("Overwrite this attack pattern's properties with the copied attack's properties?", newProperties)->sink()
				.connect<AttackPatternEditorPropertiesPanel, &AttackPatternEditorPropertiesPanel::onPasteIntoConfirmation>(this);
		} else {
			return "Cannot overwrite this attack pattern's properties when copying more than one attack pattern.";
		}
	}
	return "";
}

bool AttackPatternEditorPropertiesPanel::handleEvent(sf::Event event) {
	if (relationshipEditor->isFocused() && relationshipEditor->handleEvent(event)) {
		return true;
	} else if (event.type == sf::Event::KeyPressed) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::Z) {
				undoStack.undo();
				return true;
			} else if (event.key.code == sf::Keyboard::Y) {
				undoStack.redo();
				return true;
			} else if (event.key.code == sf::Keyboard::V) {
				clipboard.paste2(this);
				return true;
			}
		}
	}
	return false;
}

tgui::Signal& AttackPatternEditorPropertiesPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onAttackPatternModify.getName())) {
		return onAttackPatternModify;
	}
	return tgui::Panel::getSignal(signalName);
}

void AttackPatternEditorPropertiesPanel::manualPaste() {
	//TODO
}

void AttackPatternEditorPropertiesPanel::manualUndo() {
	undoStack.undo();
}

void AttackPatternEditorPropertiesPanel::manualRedo() {
	undoStack.redo();
}

void AttackPatternEditorPropertiesPanel::onPasteIntoConfirmation(bool confirmed, CopiedAttackPatternProperties newProperties) {
	if (confirmed) {
		CopiedAttackPatternProperties oldProperties(attackPattern->getName(), attackPattern->getAttacks());
		undoStack.execute(UndoableCommand([this, newProperties]() {
			attackPattern->setName(newProperties.name);
			attackPattern->setAttacks(newProperties.attacks);
			
			ignoreSignals = true;
			this->name->setText(newProperties.name);
			this->relationshipEditor->setRelationships(AttackPatternToAttackUseRelationship::convertDataVectorToRelationshipVector(newProperties.attacks));
			ignoreSignals = false;

			onAttackPatternModify.emit(this);
		}, [this, oldProperties]() {
			attackPattern->setName(oldProperties.name);
			attackPattern->setAttacks(oldProperties.attacks);

			ignoreSignals = true;
			this->name->setText(oldProperties.name);
			this->relationshipEditor->setRelationships(AttackPatternToAttackUseRelationship::convertDataVectorToRelationshipVector(oldProperties.attacks));
			ignoreSignals = false;

			onAttackPatternModify.emit(this);
		}));
	}
}

std::shared_ptr<ListViewScrollablePanel> AttackPatternEditorPropertiesPanel::getUsedByPanel() {
	return usedBy;
}

void AttackPatternEditorPropertiesPanel::setSymbolTablesHierarchy(std::vector<ValueSymbolTable> symbolTablesHierarchy) {
	relationshipEditor->setSymbolTablesHierarchy(symbolTablesHierarchy);
}
