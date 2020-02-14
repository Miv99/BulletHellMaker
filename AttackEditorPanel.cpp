#include "AttackEditorPanel.h"

AttackEditorPanel::AttackEditorPanel(EditorWindow& parentWindow, LevelPack& levelPack, std::shared_ptr<EditorAttack> attack, int undoStackSize) : levelPack(levelPack), undoStack(UndoStack(undoStackSize)), attack(attack) {
	tabs = TabsWithPanel::create(parentWindow);
	tabs->setPosition(0, 0);
	tabs->setSize("100%", "100%");
	add(tabs);

	{
		// Properties
		std::shared_ptr<tgui::ScrollablePanel> properties = tgui::ScrollablePanel::create();
		std::shared_ptr<tgui::Label> id = tgui::Label::create();
		std::shared_ptr<tgui::Label> nameLabel = tgui::Label::create();
		std::shared_ptr<tgui::EditBox> name = tgui::EditBox::create();
		std::shared_ptr<tgui::Label> usedByLabel = tgui::Label::create();
		usedBy = ListBoxScrollablePanel::create();

		id->setText("Attack ID " + std::to_string(attack->getID()));
		nameLabel->setText("Name");
		name->setText(attack->getName());
		usedByLabel->setText("Used by");

		id->setTextSize(TEXT_SIZE);
		nameLabel->setTextSize(TEXT_SIZE);
		name->setTextSize(TEXT_SIZE);
		usedByLabel->setTextSize(TEXT_SIZE);
		usedBy->getListBox()->setTextSize(TEXT_SIZE);

		id->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
		nameLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(id) + GUI_PADDING_Y);
		name->setPosition(tgui::bindLeft(id), tgui::bindBottom(nameLabel) + GUI_LABEL_PADDING_Y);
		usedByLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(name) + GUI_PADDING_Y);
		usedBy->setPosition(tgui::bindLeft(id), tgui::bindBottom(usedByLabel) + GUI_LABEL_PADDING_Y);

		properties->connect("SizeChanged", [this, name](sf::Vector2f newSize) {
			float fillX = newSize.x - GUI_PADDING_X * 2;
			name->setSize(fillX, name->getSize().y);
			this->usedBy->setSize(fillX, newSize.y - this->usedBy->getPosition().y - GUI_PADDING_Y * 2);
		});

		name->connect("ReturnKeyPressed", [this, name](std::string text) {
			std::string oldName = this->attack->getName();
			undoStack.execute(UndoableCommand([this, name, text]() {
				this->attack->setName(text);
				name->setText(text);
				onAttackModify.emit(this, this->attack);
			}, [this, name, oldName]() {
				this->attack->setName(oldName);
				name->setText(oldName);
				onAttackModify.emit(this, this->attack);
			}));
		});

		properties->add(id);
		properties->add(nameLabel);
		properties->add(name);
		properties->add(usedByLabel);
		properties->add(usedBy);
		
		tabs->addTab("Properties", properties);

		// Populate the usedBy list when the level pack is changed
		levelPack.getOnChange()->sink().connect<AttackEditorPanel, &AttackEditorPanel::populatePropertiesUsedByList>(this);
		// Initial population
		populatePropertiesUsedByList();

		usedBy->getListBox()->connect("DoubleClicked", [&](int index) {
			if (usedByIDMap.count(index) > 0) {
				onAttackPatternDoubleClick.emit(this, usedByIDMap[index]);
			}
		});
	}
	{
		// EMP tree view
		std::shared_ptr<tgui::Panel> emps = tgui::Panel::create();
		std::shared_ptr<tgui::Label> empsTreeViewLabel = tgui::Label::create();
		std::shared_ptr<tgui::TreeView> empsTreeView = tgui::TreeView::create();
	}
}

bool AttackEditorPanel::handleEvent(sf::Event event) {
	if (event.type == sf::Event::KeyPressed) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
			if (event.key.code == sf::Keyboard::Z) {
				undoStack.undo();
			} else if (event.key.code == sf::Keyboard::Y) {
				undoStack.redo();
			}
		}
	}
	return false;
}

tgui::Signal& AttackEditorPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onAttackPatternDoubleClick.getName())) {
		return onAttackPatternDoubleClick;
	} else if (signalName == tgui::toLower(onAttackModify.getName())) {
		return onAttackModify;
	}
	return tgui::Panel::getSignal(signalName);
}

void AttackEditorPanel::populatePropertiesUsedByList() {
	usedByIDMap.clear();
	usedBy->getListBox()->removeAllItems();
	auto attackPatternIDs = levelPack.getAttackUsers(attack->getID());
	for (int i = 0; i < attackPatternIDs.size(); i++) {
		int id = attackPatternIDs[i];
		usedByIDMap[i] = id;
		usedBy->getListBox()->addItem("Attack pattern ID " + std::to_string(id));
	}
	usedBy->onListBoxItemsUpdate();
}