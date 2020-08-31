#include <Editor/Attack/AttackEditorPropertiesPanel.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/EditorWindow.h>

AttackEditorPropertiesPanel::AttackEditorPropertiesPanel(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize) 
	: CopyPasteable("EditorAttack"), mainEditorWindow(mainEditorWindow), clipboard(clipboard),
	attack(attack), undoStack(UndoStack(undoStackSize)) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

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

std::pair<std::shared_ptr<CopiedObject>, std::string> AttackEditorPropertiesPanel::copyFrom() {
	// Can't copy this widget
	return std::make_pair(nullptr, "");
}

std::string AttackEditorPropertiesPanel::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	// Same functionality as paste2Into()
	return paste2Into(pastedObject);
}

std::string AttackEditorPropertiesPanel::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	// Paste the first copied EditorAttack to override attack's properties
	auto derived = std::static_pointer_cast<CopiedLevelPackObject>(pastedObject);
	if (derived) {
		std::vector<std::shared_ptr<EditorAttack>> copiedAttacks;
		for (auto obj : derived->getLevelPackObjects()) {
			copiedAttacks.push_back(std::dynamic_pointer_cast<EditorAttack>(obj));
		}
		if (copiedAttacks.size() == 1) {
			std::shared_ptr<EditorAttack> newAttack = std::make_shared<EditorAttack>(attack);
			newAttack->setName(copiedAttacks[0]->getName());

			mainEditorWindow.promptConfirmation("Overwrite this attack's properties with the copied attack's properties?", newAttack)->sink()
				.connect<AttackEditorPropertiesPanel, &AttackEditorPropertiesPanel::onPasteIntoConfirmation>(this);
		} else {
			return "Cannot overwrite this attack's properties when copying more than one attack.";
		}
	}
	return "";
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

tgui::Signal& AttackEditorPropertiesPanel::getSignal(std::string signalName) {
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
