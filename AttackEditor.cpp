#include "AttackEditor.h"
#include <iostream>
#include "Constants.h"
#include "TextMarshallable.h"

AttackEditor::AttackEditor(LevelPack& levelPack, std::shared_ptr<SpriteLoader> spriteLoader) : levelPack(levelPack), spriteLoader(spriteLoader) {
	tguiMutex = std::make_shared<std::mutex>();

	mainWindow = std::make_shared<EditorWindow>(tguiMutex, "Attack Editor", MAIN_WINDOW_WIDTH, 400);
	playAreaWindow = std::make_shared<EditorWindow>(tguiMutex, "Attack Editor - Gameplay Test", MAP_WIDTH, MAP_HEIGHT);

	std::lock_guard<std::mutex> lock(*tguiMutex);

	//------------------ Attack info window widgets (ai__) ---------------------
	aiPanel = tgui::ScrollablePanel::create();
	aiID = tgui::Label::create();
	aiName = tgui::EditBox::create();
	aiPlayAttackAnimation = tgui::CheckBox::create();

	aiID->setTextSize(16);
	aiName->setTextSize(16);

	const float aiAreaWidth = mainWindow->getWindowWidth() * 0.25f;
	aiPanel->setPosition(0, 0);
	aiID->setMaximumTextWidth(aiAreaWidth - GUI_PADDING_X * 2);
	aiPlayAttackAnimation->setSize(25, 25);
	aiName->setSize(aiAreaWidth - GUI_PADDING_X * 2, 20);
	aiID->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	aiName->setPosition({ tgui::bindLeft(aiID), tgui::bindBottom(aiID) + GUI_PADDING_Y });
	aiPlayAttackAnimation->setPosition({ tgui::bindLeft(aiID), tgui::bindBottom(aiName) + GUI_PADDING_Y });
	aiPanel->setSize(aiAreaWidth, std::min(mainWindow->getWindowHeight()*0.25f, aiPlayAttackAnimation->getPosition().y + aiPlayAttackAnimation->getSize().y + GUI_PADDING_Y));

	aiID->setText("No attack selected");
	aiName->setReadOnly(true);
	// Name can be anything but the TextMarshallable delimiter
	aiName->setInputValidator("^[^" + tos(DELIMITER) + "]+$");
	aiPlayAttackAnimation->setText("Enable attack animation");
	aiPlayAttackAnimation->setChecked(false);
	aiPlayAttackAnimation->setTextClickable(true);

	aiName->connect("ReturnKeyPressed", [&](std::string text) {
		if (selectedAttack) {
			bool attackChanged = text != selectedAttack->getName();
			selectedAttack->setName(text);
			alList->changeItemById(std::to_string(selectedAttack->getID()), text + " [id=" + std::to_string(selectedAttack->getID()) + "]");
			if (attackChanged) {
				onAttackChange(selectedAttack);
			}
		}
	});
	aiPlayAttackAnimation->connect("Changed", [&]() {
		if (selectedAttack) {
			bool checked = aiPlayAttackAnimation->isChecked();
			bool attackChanged = selectedAttack->getPlayAttackAnimation() != checked;
			selectedAttack->setPlayAttackAnimation(checked);
			if (attackChanged) {
				onAttackChange(selectedAttack);
			}
		}
	});

	aiID->setToolTip(createToolTip("The ID of an attack is used to uniquely identify the attack."));
	aiName->setToolTip(createToolTip("The name of an attack is optional but recommended. It helps you identify what the attack is or does."));
	aiPlayAttackAnimation->setToolTip(createToolTip("If this is checked, when an enemy executes this attack, it will play its attack animation. Otherwise, it will execute the attack but not play its attack animation."));

	aiPanel->add(aiID);
	aiPanel->add(aiName);
	aiPanel->add(aiPlayAttackAnimation);
	//------------------ Attack list window widgets --------------------------------
	alPanel = tgui::ScrollablePanel::create();
	alList = tgui::ListBox::create();
	alSaveAll = tgui::Button::create();
	alDiscardAll = tgui::Button::create();
	alCreateAttack = tgui::Button::create();
	alDeleteAttack = tgui::Button::create();

	alList->setTextSize(16);
	alList->setItemHeight(20);
	alList->setMaximumItems(0);

	alSaveAll->setTextSize(16);
	alDiscardAll->setTextSize(16);

	const float alAreaWidth = mainWindow->getWindowWidth() * 0.25f;
	const float alAreaHeight = mainWindow->getWindowHeight() - aiPanel->getPosition().y - aiPanel->getSize().y;
	alPanel->setSize(alAreaWidth, alAreaHeight);
	alPanel->setPosition(0, tgui::bindBottom(aiPanel));
	alSaveAll->setSize(100, 25);
	alDiscardAll->setSize(100, 25);
	alCreateAttack->setSize(100, 25);
	alDeleteAttack->setSize(100, 25);
	alList->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	alCreateAttack->setPosition(tgui::bindLeft(alList), alAreaHeight - alCreateAttack->getSize().y - GUI_PADDING_Y);
	alDeleteAttack->setPosition(tgui::bindRight(alCreateAttack) + GUI_PADDING_X, tgui::bindTop(alCreateAttack));
	alSaveAll->setPosition(tgui::bindLeft(alList), tgui::bindTop(alCreateAttack) - alSaveAll->getSize().y - GUI_PADDING_Y);
	alDiscardAll->setPosition(tgui::bindRight(alSaveAll) + GUI_PADDING_X, tgui::bindTop(alSaveAll));
	alList->setSize(alAreaWidth - GUI_PADDING_X * 2, alSaveAll->getPosition().y - GUI_PADDING_Y * 2);

	alSaveAll->setText("Save all");
	alDiscardAll->setText("Discard all");
	alCreateAttack->setText("New");
	alDeleteAttack->setText("Delete");
	alDeleteAttack->setEnabled(false);

	// Disable autoscroll on initial populating so that alList is looking at the first attack on startup
	alList->setAutoScroll(false);
	for (auto it = levelPack.getAttackIteratorBegin(); it != levelPack.getAttackIteratorEnd(); it++) {
		alList->addItem(it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
	}
	alList->setAutoScroll(true);

	alSaveAll->setToolTip(createToolTip("Save all changes made to all attacks. Attacks with unsaved changes are denoted by an asterisk (*)"));
	alDiscardAll->setToolTip(createToolTip("Discard all changes made to all attacks. Attacks with unsaved changes are denoted by an asterisk (*)"));
	alCreateAttack->setToolTip(createToolTip("Create a new attack"));
	alDeleteAttack->setToolTip(createToolTip("Delete the selected attack"));

	alList->connect("ItemSelected", [&](std::string itemName, std::string itemID) {
		if (itemID != "") {
			selectAttack(std::stoi(itemID));
		}
	});
	alSaveAll->connect("Pressed", [&]() { 
		std::vector<std::shared_ptr<EditorAttack>> queued;
		for (auto it = unsavedAttacks.begin(); it != unsavedAttacks.end(); it++) {
			queued.push_back(it->second);
		}
		for (auto attack : queued) {
			saveAttack(attack);
		}
	});
	alDiscardAll->connect("Pressed", [&]() {
		std::vector<std::shared_ptr<EditorAttack>> queued;
		for (auto it = unsavedAttacks.begin(); it != unsavedAttacks.end(); it++) {
			queued.push_back(it->second);
		}
		for (auto attack : queued) {
			discardAttackChanges(attack);
		}
	});
	alCreateAttack->connect("Pressed", [&]() {
		createAttack();
	});
	alDeleteAttack->connect("Pressed", [&]() {
		if (selectedAttack) {
			deleteAttack(selectedAttack);
		}
	});

	alPanel->add(alList);
	alPanel->add(alSaveAll);
	alPanel->add(alDiscardAll);
	alPanel->add(alCreateAttack);
	alPanel->add(alDeleteAttack);

	//TODO: for attack list, put a * at the beginning of any attacks with unsaved changes
	// User can right click the attack and choose: save/discard changes.
	// If user control+s to save, only the currently selected attack is saved.
	// Attack list will have a button to save all and discard all.

	//------------------ EMP list window widgets --------------------------------
	emplPanel = tgui::ScrollablePanel::create();
	emplTree = tgui::TreeView::create();
	emplLabel = tgui::Label::create();
	emplCreateEMP = tgui::Button::create();
	emplDeleteEMP = tgui::Button::create();

	emplTree->getRenderer()->setBackgroundColor(sf::Color(190, 190, 190, 255));
	emplTree->setItemHeight(20);

	emplLabel->setMaximumTextWidth(0);
	emplLabel->setTextSize(16);
	emplLabel->setText("Movable points");

	const float emplAreaWidth = mainWindow->getWindowWidth() * 0.5f;
	emplPanel->setPosition(tgui::bindRight(aiPanel), 0);
	emplPanel->setSize(emplAreaWidth, mainWindow->getWindowHeight());
	emplLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	emplCreateEMP->setSize(100, 25);
	emplDeleteEMP->setSize(100, 25);
	emplCreateEMP->setPosition(tgui::bindLeft(emplTree), mainWindow->getWindowHeight() - emplCreateEMP->getSize().y - GUI_PADDING_Y);
	emplDeleteEMP->setPosition(tgui::bindRight(emplCreateEMP) + GUI_PADDING_X, tgui::bindTop(emplCreateEMP));
	emplTree->setPosition(tgui::bindLeft(emplLabel), tgui::bindBottom(emplLabel) + GUI_PADDING_Y);
	emplTree->setSize(emplAreaWidth - GUI_PADDING_X * 2, emplCreateEMP->getPosition().y - emplLabel->getPosition().y - emplLabel->getSize().y - GUI_PADDING_Y * 2);

	emplCreateEMP->setText("New");
	emplDeleteEMP->setText("Delete");

	emplCreateEMP->setToolTip(createToolTip("Create a new movable point"));
	emplDeleteEMP->setToolTip(createToolTip("Delete the selected movable point"));
	
	emplTree->connect("ItemSelected", [&](sf::String nodeText) {
		if (nodeText != "") {
			std::string str = nodeText;
			// Extract the "[id=___]" portion out of the node text
			int idBeginPos = str.find_last_of('=') + 1;
			selectEMP(std::stoi(str.substr(idBeginPos, str.length() - idBeginPos - 1)));
		}
	});
	emplCreateEMP->connect("Pressed", [&]() {
		createEMP(selectedAttack, selectedEMP);
	});
	emplDeleteEMP->connect("Pressed", [&]() {
		// Can't delete main EMP of an attack
		if (selectedEMP && selectedEMP->getID() != selectedAttack->getMainEMP()->getID()) {
			deleteEMP(selectedAttack, selectedEMP);
		}
	});

	emplPanel->add(emplLabel);
	emplPanel->add(emplTree);
	emplPanel->add(emplCreateEMP);
	emplPanel->add(emplDeleteEMP);
	//------------------ EMP info window widgets --------------------------------
	empiPanel = tgui::ScrollablePanel::create();
	empiAnimatableLabel = tgui::Label::create();
	empiAnimatable = std::make_shared<AnimatableChooser>(*spriteLoader, false);

	empiAnimatable->setTextSize(16);
	empiAnimatableLabel->setTextSize(16);

	empiAnimatableLabel->setText("Sprite/Animation");

	const float empiAreaWidth = mainWindow->getWindowWidth() * 0.25f;
	empiPanel->setPosition(tgui::bindRight(emplPanel), 0);
	empiPanel->setSize(empiAreaWidth, mainWindow->getWindowHeight());
	empiAnimatableLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	empiAnimatableLabel->setMaximumTextWidth(0);
	auto empiAnimatablePicture = empiAnimatable->getAnimatablePicture();
	empiAnimatablePicture->setSize(100, 100);
	empiAnimatablePicture->setPosition(tgui::bindLeft(empiAnimatableLabel), tgui::bindBottom(empiAnimatableLabel) + GUI_PADDING_Y);
	empiAnimatable->setSize(empiAreaWidth - GUI_PADDING_X * 2, 20);
	empiAnimatable->setPosition(tgui::bindLeft(empiAnimatableLabel), tgui::bindBottom(empiAnimatablePicture) + GUI_PADDING_Y);

	empiPanel->add(empiAnimatableLabel);
	empiPanel->add(empiAnimatable);
	empiPanel->add(empiAnimatablePicture);
	//TODO: call this and empiBaseSprite->calculateItemsToDisplay() every time window size changes
	empiAnimatable->calculateItemsToDisplay();

	//------------------ Play area window widgets --------------------------------

}

void AttackEditor::start() {
	mainWindowThread = std::thread(&EditorWindow::start, mainWindow);
	playAreaWindowThread = std::thread(&EditorWindow::start, playAreaWindow);

	mainWindowThread.detach();
	playAreaWindowThread.detach();

	mainWindow->getRenderSignal()->sink().connect<AttackEditor, &AttackEditor::onMainWindowRender>(this);

	std::shared_ptr<tgui::Gui> mainWindowGui = mainWindow->getGui();
	mainWindowGui->add(aiPanel);
	mainWindowGui->add(alPanel);
	mainWindowGui->add(emplPanel);
	mainWindowGui->add(empiPanel);

	deselectAttack();
}

void AttackEditor::close() {
	//TODO: if there are unsaved changes, create a "are you sure" prompt first
}

void AttackEditor::selectAttack(int id) {
	if (unsavedAttacks.count(id) > 0) {
		selectedAttack = unsavedAttacks[id];
	} else {
		selectedAttack = std::make_shared<EditorAttack>(levelPack.getAttack(id));
	}

	aiID->setText("Attack ID: " + std::to_string(id));
	aiName->setReadOnly(false);
	aiName->setText(selectedAttack->getName());
	aiPlayAttackAnimation->setChecked(selectedAttack->getPlayAttackAnimation());
	buildEMPTree();
	alDeleteAttack->setEnabled(true);
}

void AttackEditor::deselectAttack() {
	deselectEMP();
	selectedAttack = nullptr;

	aiID->setText("No attack selected");
	aiName->setText("");
	aiName->setReadOnly(true);
	aiPlayAttackAnimation->setChecked(false);
	emplTree->removeAllItems();
	alList->deselectItem();
	alDeleteAttack->setEnabled(false);
}

void AttackEditor::selectEMP(int empID) {
	selectedEMP = selectedAttack->searchEMP(empID);

	emplCreateEMP->setEnabled(true);
	// Can't delete main EMP of an attack
	emplDeleteEMP->setEnabled(empID != selectedAttack->getMainEMP()->getID());

	// TODO: all the empi stuff
}

void AttackEditor::deselectEMP() {
	selectedEMP = nullptr;

	emplCreateEMP->setEnabled(false);
	emplDeleteEMP->setEnabled(false);

	//TODO: clear all the empi stuff
}

void AttackEditor::createAttack() {
	std::shared_ptr<EditorAttack> attack = levelPack.createAttack();
	alList->addItem(attack->getName() + " [id=" + std::to_string(attack->getID()) + "]", std::to_string(attack->getID()));
}

void AttackEditor::deleteAttack(std::shared_ptr<EditorAttack> attack) {
	//TODO: prompt: are you sure? this attack may be in use by some attack patterns.

	if (selectedAttack->getID() == attack->getID()) {
		deselectAttack();
	}
	alList->removeItemById(std::to_string(attack->getID()));
	
	if (unsavedAttacks.count(attack->getID()) > 0) {
		unsavedAttacks.erase(attack->getID());
	}

	levelPack.deleteAttack(attack->getID());
}

void AttackEditor::saveAttack(std::shared_ptr<EditorAttack> attack) {
	levelPack.updateAttack(attack);

	int id = attack->getID();
	if (alList->containsId(std::to_string(id))) {
		alList->changeItemById(std::to_string(id), attack->getName() + " [id=" + std::to_string(id) + "]");
	}
	if (unsavedAttacks.count(id) > 0) {
		unsavedAttacks.erase(id);
	}
}

void AttackEditor::discardAttackChanges(std::shared_ptr<EditorAttack> attack) {
	unsavedAttacks.erase(attack->getID());

	std::shared_ptr<EditorAttack> revertedAttack = levelPack.getAttack(attack->getID());

	int id = attack->getID();
	if (alList->containsId(std::to_string(id))) {
		alList->changeItemById(std::to_string(id), revertedAttack->getName() + " [id=" + std::to_string(id) + "]");
	}
	if (selectedAttack->getID() == id) {
		deselectAttack();
	}
}

void AttackEditor::createEMP(std::shared_ptr<EditorAttack> empOwner, std::shared_ptr<EditorMovablePoint> parent) {
	std::shared_ptr<EditorMovablePoint> emp = parent->createChild();
	
	emplTree->addItem(emp->generatePathToThisEmp(&AttackEditor::getEMPTreeNodeText));

	onAttackChange(empOwner);
}

void AttackEditor::deleteEMP(std::shared_ptr<EditorAttack> empOwner, std::shared_ptr<EditorMovablePoint> emp) {
	assert(emp->getID() != empOwner->getMainEMP()->getID());

	emp->detachFromParent();
	buildEMPTree();

	onAttackChange(empOwner);
}

void AttackEditor::onAttackChange(std::shared_ptr<EditorAttack> attackWithUnsavedChanges) {
	int id = attackWithUnsavedChanges->getID();
	unsavedAttacks[id] = attackWithUnsavedChanges;
	
	// Add asterisk to the entry in attack list
	if (unsavedAttacks.count(id) > 0) {
		alList->changeItemById(std::to_string(id), "*" + attackWithUnsavedChanges ->getName() + " [id=" + std::to_string(id) + "]");
	}
}

void AttackEditor::buildEMPTree() {
	emplTree->removeAllItems();
	for (std::vector<sf::String> item : selectedAttack->generateTreeViewEmpHierarchy(&AttackEditor::getEMPTreeNodeText)) {
		emplTree->addItem(item);
	}

	emplTree->deselectItem();
}

sf::String AttackEditor::getEMPTreeNodeText(const EditorMovablePoint& emp) {
	//TODO: inidicate if it's a bullet and other stuff
	// Whether EMP is a bullet or just an anchor (a reference point; not a bullet)
	std::string bulletStr = emp.isBullet() ? "[B]" : "[A]";
	std::string idStr = "[id=" + std::to_string(emp.getID()) + "]";
	return bulletStr + " " + idStr;
}

void AttackEditor::onMainWindowRender(float deltaTime) {
	std::lock_guard<std::mutex> lock(*tguiMutex);

	empiAnimatable->getAnimatablePicture()->update(deltaTime);
	//TODO: add the other animatable stuff
}
