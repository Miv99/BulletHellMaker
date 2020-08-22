#include <Editor/AttackPattern/AttackPatternEditorPropertiesPanel.h>

#include <GuiConfig.h>
#include <Editor/EditorWindow.h>

AttackPatternEditorPropertiesPanel::AttackPatternEditorPropertiesPanel(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::shared_ptr<EditorAttackPattern> attackPattern, int undoStackSize)
	: CopyPasteable("EditorAttackPattern"), mainEditorWindow(mainEditorWindow), clipboard(clipboard), attackPattern(attackPattern), undoStack(UndoStack(undoStackSize)) {
	std::shared_ptr<tgui::Label> id = tgui::Label::create();
	std::shared_ptr<tgui::Label> nameLabel = tgui::Label::create();
	name = EditBox::create();
	std::shared_ptr<tgui::Label> usedByLabel = tgui::Label::create();
	usedBy = ListViewScrollablePanel::create();

	id->setText("Attack pattern ID " + std::to_string(attackPattern->getID()));
	nameLabel->setText("Name");
	name->setText(attackPattern->getName());
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

	add(id);
	add(nameLabel);
	add(name);
	add(usedByLabel);
	add(usedBy);
}

std::pair<std::shared_ptr<CopiedObject>, std::string> AttackPatternEditorPropertiesPanel::copyFrom() {
	//TODO
	return std::pair<std::shared_ptr<CopiedObject>, std::string>();
}

std::string AttackPatternEditorPropertiesPanel::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	//TODO
	return std::string();
}

std::string AttackPatternEditorPropertiesPanel::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	//TODO
	return std::string();
}

bool AttackPatternEditorPropertiesPanel::handleEvent(sf::Event event) {
	//TODO
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

std::shared_ptr<ListViewScrollablePanel> AttackPatternEditorPropertiesPanel::getUsedByPanel() {
	return usedBy;
}
