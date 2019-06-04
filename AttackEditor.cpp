#include "AttackEditor.h"
#include <iostream>
#include "Constants.h"
#include "TextMarshallable.h"

std::shared_ptr<tgui::Label> createTooltip(std::string text) {
	auto tooltip = tgui::Label::create();
	tooltip->setMaximumTextWidth(300);
	tooltip->setText(text);
	tooltip->setTextSize(12);
	tooltip->getRenderer()->setBackgroundColor(tgui::Color::White);

	return tooltip;
}

AttackEditor::AttackEditor(LevelPack& levelPack) : levelPack(levelPack) {
	tguiMutex = std::make_shared<std::mutex>();

	attackInfoWindow = std::make_shared<EditorWindow>(tguiMutex, "Attack Editor - Attack Info", AI_WINDOW_WIDTH, AI_WINDOW_HEIGHT);
	empInfoWindow = std::make_shared<EditorWindow>(tguiMutex, "Attack Editor - Movable Point Info", 400, 768);
	attackListWindow = std::make_shared<EditorWindow>(tguiMutex, "Attack Editor - Attacks List", 250, 768);
	empListWindow = std::make_shared<EditorWindow>(tguiMutex, "Attack Editor - Movable Points List", 500, 768);
	playAreaWindow = std::make_shared<EditorWindow>(tguiMutex, "Attack Editor - Gameplay Test", MAP_WIDTH, MAP_HEIGHT);

	std::lock_guard<std::mutex> lock(*tguiMutex);

	//------------------ Attack info window widgets (ai__) ---------------------
	aiId = tgui::Label::create();
	aiName = tgui::EditBox::create();
	aiPlayAttackAnimation = tgui::CheckBox::create();

	aiId->setTextSize(24);
	aiId->setMaximumTextWidth(attackInfoWindow->getWindowWidth() - GUI_PADDING_X * 2);
	aiName->setTextSize(24);
	aiName->setSize(attackInfoWindow->getWindowWidth() - GUI_PADDING_X * 2, 28);
	aiPlayAttackAnimation->setSize(25, 25);

	aiId->setText("No attack selected");
	aiName->setReadOnly(true);
	// Name can be anything but the TextMarshallable delimiter
	aiName->setInputValidator("^[^" + tos(DELIMITER) + "]+$");
	aiPlayAttackAnimation->setText("Enable attack animation");
	aiPlayAttackAnimation->setChecked(false);
	aiPlayAttackAnimation->setTextClickable(true);

	aiId->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	aiName->setPosition({ tgui::bindLeft(aiId), tgui::bindBottom(aiId) + GUI_PADDING_Y });
	aiPlayAttackAnimation->setPosition({ tgui::bindLeft(aiId), tgui::bindBottom(aiName) + GUI_PADDING_Y });

	aiId->setToolTip(createTooltip("The ID of an attack is used to uniquely identify the attack."));
	aiName->setToolTip(createTooltip("The name of an attack is optional but recommended. It helps you identify what the attack is or does."));
	aiPlayAttackAnimation->setToolTip(createTooltip("If this is checked, when an enemy executes this attack, it will play its attack animation. Otherwise, it will execute the attack but not play its attack animation."));

	//------------------ EMP info window widgets --------------------------------
	
	
	//------------------ Attack list window widgets --------------------------------

	alList = tgui::ListBox::create();
	alSaveAll = tgui::Button::create();
	alDiscardAll = tgui::Button::create();

	alList->setTextSize(16);
	alList->setItemHeight(20);
	alList->setMaximumItems(0);
	alList->setAutoScroll(false);

	alSaveAll->setSize(100, 25);
	alSaveAll->setPosition(GUI_PADDING_X, attackListWindow->getWindowHeight() - GUI_PADDING_Y - alSaveAll->getSize().y);
	alSaveAll->setTextSize(16);
	alDiscardAll->setSize(100, 25);
	alDiscardAll->setPosition(tgui::bindRight(alSaveAll) + GUI_PADDING_X, attackListWindow->getWindowHeight() - GUI_PADDING_Y - alDiscardAll->getSize().y);
	alDiscardAll->setTextSize(16);
	alList->setSize(attackListWindow->getWindowWidth() - GUI_PADDING_X * 2, alSaveAll->getPosition().y - GUI_PADDING_Y);
	alList->setPosition(GUI_PADDING_X, GUI_PADDING_Y);

	alSaveAll->setText("Save all");
	alDiscardAll->setText("Discard all");
	
	for (auto it = levelPack.getAttackIteratorBegin(); it != levelPack.getAttackIteratorEnd(); it++) {
		alList->addItem(it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
	}
	//TODO: for attack list, put a * at the beginning of any attacks with unsaved changes
	// User can right click the attack and choose: save/discard changes.
	// If user control+s to save, only the currently selected attack is saved.
	// Attack list will have a button to save all and discard all.

	//------------------ EMP list window widgets --------------------------------
	
	
	//------------------ Play area window widgets --------------------------------

}

void AttackEditor::start() {
	attackInfoWindowThread = std::thread(&EditorWindow::start, attackInfoWindow);
	empInfoWindowThread = std::thread(&EditorWindow::start, empInfoWindow);
	attackListWindowThread = std::thread(&EditorWindow::start, attackListWindow);
	empListWindowThread = std::thread(&EditorWindow::start, empListWindow);
	playAreaWindowThread = std::thread(&EditorWindow::start, playAreaWindow);

	attackInfoWindowThread.detach();
	empInfoWindowThread.detach();
	attackListWindowThread.detach();
	empListWindowThread.detach();
	playAreaWindowThread.detach();

	std::shared_ptr<tgui::Gui> aiGui = attackInfoWindow->getGui();
	aiGui->add(aiId);
	aiGui->add(aiName);
	aiGui->add(aiPlayAttackAnimation);

	std::shared_ptr<tgui::Gui> alGui = attackListWindow->getGui();
	alGui->add(alList);
	alGui->add(alSaveAll);
	alGui->add(alDiscardAll);
}

void AttackEditor::close() {
	//TODO: if there are unsaved changes, create a "are you sure" prompt first
}

void AttackEditor::selectAttack(int id) {
	deselectAttack();

	selectedAttack = std::make_shared<EditorAttack>(levelPack.getAttack(id));

	aiId->setText("Attack ID: " + std::to_string(id));
	aiName->setReadOnly(false);
	aiName->setText(selectedAttack->getName());
	aiPlayAttackAnimation->setChecked(selectedAttack->getPlayAttackAnimation());
}

void AttackEditor::deselectAttack() {
	deselectEMP();

	aiId->setText("No attack selected");
	aiName->setReadOnly(true);
	aiPlayAttackAnimation->setChecked(false);
}

void AttackEditor::deselectEMP() {
}

void AttackEditor::onAttackChange(std::shared_ptr<EditorAttack> attackWithUnsavedChanges) {
	int id = attackWithUnsavedChanges->getID();
	attackHasUnsavedChanges[id] = attackWithUnsavedChanges;
	
	// Add asterisk to the entry in attack list
	if (!attackHasUnsavedChanges[id]) {
		alList->changeItemById(std::to_string(id), "*" + alList->getItemById(std::to_string(id)));
	}
}
