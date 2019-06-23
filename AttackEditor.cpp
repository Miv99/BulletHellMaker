#include "AttackEditor.h"
#include <iostream>
#include "Constants.h"
#include "TextMarshallable.h"
#include "CollisionSystem.h"

std::string getID(BULLET_ON_COLLISION_ACTION onCollisionAction) {
	return std::to_string(static_cast<int>(onCollisionAction));
}

std::string getID(std::shared_ptr<EMPSpawnType> spawnType) {
	if (dynamic_cast<SpecificGlobalEMPSpawn*>(spawnType.get())) {
		return "0";
	} else if (dynamic_cast<EntityRelativeEMPSpawn*>(spawnType.get())) {
		return "1";
	} else if (dynamic_cast<EntityAttachedEMPSpawn*>(spawnType.get())) {
		return "2";
	}
}

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

	aiID->setTextSize(TEXT_SIZE);
	aiName->setTextSize(TEXT_SIZE);

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

	alList->setTextSize(TEXT_SIZE);
	alList->setItemHeight(TEXT_BOX_HEIGHT);
	alList->setMaximumItems(0);

	alSaveAll->setTextSize(TEXT_SIZE);
	alDiscardAll->setTextSize(TEXT_SIZE);

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
	emplTree->setItemHeight(TEXT_BOX_HEIGHT);

	emplLabel->setMaximumTextWidth(0);
	emplLabel->setTextSize(TEXT_SIZE);
	emplLabel->setText("Movable points");

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
		} else {
			deselectEMP();
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
	empiID = tgui::Label::create();
	empiIsBulletLabel = tgui::Label::create();
	empiIsBullet = tgui::CheckBox::create();
	empiHitboxRadiusLabel = tgui::Label::create();
	empiHitboxRadius = std::make_shared<SliderWithEditBox>();
	empiDespawnTimeLabel = tgui::Label::create();
	empiDespawnTime = std::make_shared<SliderWithEditBox>();
	empiActionsLabel = tgui::Label::create();
	empiActions = tgui::ListBox::create();
	empiSpawnTypeLabel = tgui::Label::create();

	empiSpawnType = tgui::ComboBox::create();
	empiSpawnTypeTimeLabel = tgui::Label::create();
	empiSpawnTypeTime = std::make_shared<NumericalEditBoxWithLimits>();
	empiSpawnTypeXLabel = tgui::Label::create();
	empiSpawnTypeX = std::make_shared<NumericalEditBoxWithLimits>();
	empiSpawnTypeYLabel = tgui::Label::create();
	empiSpawnTypeY = std::make_shared<NumericalEditBoxWithLimits>();
	empiShadowTrailLifespanLabel = tgui::Label::create();
	empiShadowTrailLifespan = std::make_shared<SliderWithEditBox>();
	empiShadowTrailIntervalLabel = tgui::Label::create();
	empiShadowTrailInterval = std::make_shared<SliderWithEditBox>();
	empiAnimatableLabel = tgui::Label::create();
	empiAnimatable = std::make_shared<AnimatableChooser>(*spriteLoader, false);	
	empiLoopAnimationLabel = tgui::Label::create();
	empiLoopAnimation = tgui::CheckBox::create();
	empiBaseSpriteLabel = tgui::Label::create();
	empiBaseSprite = std::make_shared<AnimatableChooser>(*spriteLoader, false);
	empiDamageLabel = tgui::Label::create();
	empiDamage = std::make_shared<NumericalEditBoxWithLimits>();
	empiOnCollisionActionLabel = tgui::Label::create();
	empiOnCollisionAction = tgui::ComboBox::create();
	empiPierceResetTimeLabel = tgui::Label::create();
	empiPierceResetTime = std::make_shared<SliderWithEditBox>();
	empiSoundSettings = std::make_shared<SoundSettingsGroup>(GUI_PADDING_X, GUI_PADDING_Y);
	empiBulletModelLabel = tgui::Label::create();
	empiBulletModel = tgui::ComboBox::create();
	empiInheritRadiusLabel = tgui::Label::create();
	empiInheritRadiusLabel = tgui::Label::create();
	empiInheritRadius = tgui::CheckBox::create();
	empiInheritDespawnTimeLabel = tgui::Label::create();
	empiInheritDespawnTime = tgui::CheckBox::create();
	empiInheritShadowTrailIntervalLabel = tgui::Label::create();
	empiInheritShadowTrailInterval = tgui::CheckBox::create();
	empiInheritShadowTrailLifespanLabel = tgui::Label::create();
	empiInheritShadowTrailLifespan = tgui::CheckBox::create();
	empiInheritAnimatablesLabel = tgui::Label::create();
	empiInheritAnimatables = tgui::CheckBox::create();
	empiInheritDamageLabel = tgui::Label::create();
	empiInheritDamage = tgui::CheckBox::create();
	empiInheritSoundSettingsLabel = tgui::Label::create();
	empiInheritSoundSettings = tgui::CheckBox::create();

	empiActions->setItemHeight(20);

	empiID->setTextSize(TEXT_SIZE);
	empiIsBulletLabel->setTextSize(TEXT_SIZE);

	empiHitboxRadiusLabel->setTextSize(TEXT_SIZE);
	empiDespawnTimeLabel->setTextSize(TEXT_SIZE);
	empiActionsLabel->setTextSize(TEXT_SIZE);
	empiSpawnTypeLabel->setTextSize(TEXT_SIZE);
	empiSpawnType->setTextSize(TEXT_SIZE);
	empiSpawnTypeTimeLabel->setTextSize(TEXT_SIZE);
	empiSpawnTypeTime->setTextSize(TEXT_SIZE);
	empiSpawnTypeXLabel->setTextSize(TEXT_SIZE);
	empiSpawnTypeX->setTextSize(TEXT_SIZE);
	empiSpawnTypeYLabel->setTextSize(TEXT_SIZE);
	empiSpawnTypeY->setTextSize(TEXT_SIZE);
	empiShadowTrailLifespanLabel->setTextSize(TEXT_SIZE);
	empiShadowTrailLifespan->getEditBox()->setTextSize(TEXT_SIZE);
	empiShadowTrailIntervalLabel->setTextSize(TEXT_SIZE);
	empiShadowTrailInterval->getEditBox()->setTextSize(TEXT_SIZE);
	empiAnimatableLabel->setTextSize(TEXT_SIZE);
	empiLoopAnimationLabel->setTextSize(TEXT_SIZE);
	empiBaseSpriteLabel->setTextSize(TEXT_SIZE);
	empiDamageLabel->setTextSize(TEXT_SIZE);
	empiDamage->setTextSize(TEXT_SIZE);
	empiOnCollisionActionLabel->setTextSize(TEXT_SIZE);
	empiOnCollisionAction->setTextSize(TEXT_SIZE);
	empiPierceResetTimeLabel->setTextSize(TEXT_SIZE);
	empiPierceResetTime->getEditBox()->setTextSize(TEXT_SIZE);
	empiBulletModelLabel->setTextSize(TEXT_SIZE);
	empiBulletModel->setTextSize(TEXT_SIZE);
	empiInheritRadiusLabel->setTextSize(TEXT_SIZE);
	empiInheritDespawnTimeLabel->setTextSize(TEXT_SIZE);
	empiInheritShadowTrailIntervalLabel->setTextSize(TEXT_SIZE);
	empiInheritShadowTrailLifespanLabel->setTextSize(TEXT_SIZE);
	empiInheritAnimatablesLabel->setTextSize(TEXT_SIZE);
	empiInheritDamageLabel->setTextSize(TEXT_SIZE);
	empiInheritSoundSettingsLabel->setTextSize(TEXT_SIZE);

	empiIsBulletLabel->setText("Is bullet");
	empiHitboxRadiusLabel->setText("Hitbox radius");
	empiDespawnTimeLabel->setText("Lifespan");
	empiActionsLabel->setText("Movement actions");
	empiSpawnTypeLabel->setText("Spawn type");
	empiSpawnTypeTimeLabel->setText("Spawn delay");
	empiSpawnTypeXLabel->setText("Spawn X");
	empiSpawnTypeYLabel->setText("Spawn Y");
	empiShadowTrailLifespanLabel->setText("Shadow lifespan");
	empiShadowTrailIntervalLabel->setText("Shadow interval");
	empiAnimatableLabel->setText("Sprite/Animation");
	empiLoopAnimationLabel->setText("Loop animation");
	empiBaseSpriteLabel->setText("Final sprite");
	empiDamageLabel->setText("Damage");
	empiOnCollisionActionLabel->setText("On-collision action");
	empiPierceResetTimeLabel->setText("Pierce reset time");
	empiBulletModelLabel->setText("Bullet model");
	empiInheritRadiusLabel->setText("Inherit radius");
	empiInheritDespawnTimeLabel->setText("Inherit lifespan");
	empiInheritShadowTrailIntervalLabel->setText("Inherit shadow interval");
	empiInheritShadowTrailLifespanLabel->setText("Inherit shadow lifespan");
	empiInheritAnimatablesLabel->setText("Inherit sprites/animations");
	empiInheritDamageLabel->setText("Inherit damage");
	empiInheritSoundSettingsLabel->setText("Inherit sound settings");

	empiPanel->add(empiID);
	empiPanel->add(empiIsBulletLabel);
	empiPanel->add(empiIsBullet);
	empiPanel->add(empiHitboxRadiusLabel);
	empiPanel->add(empiHitboxRadius);
	empiPanel->add(empiHitboxRadius->getEditBox());
	empiPanel->add(empiDespawnTimeLabel);
	empiPanel->add(empiDespawnTime);
	empiPanel->add(empiDespawnTime->getEditBox());
	empiPanel->add(empiActionsLabel);
	empiPanel->add(empiActions);
	empiPanel->add(empiSpawnTypeLabel);
	empiPanel->add(empiSpawnType);
	empiPanel->add(empiSpawnTypeTimeLabel);
	empiPanel->add(empiSpawnTypeTime);
	empiPanel->add(empiSpawnTypeXLabel);
	empiPanel->add(empiSpawnTypeX);
	empiPanel->add(empiSpawnTypeYLabel);
	empiPanel->add(empiSpawnTypeY);
	empiPanel->add(empiShadowTrailLifespanLabel);
	empiPanel->add(empiShadowTrailLifespan);
	empiPanel->add(empiShadowTrailLifespan->getEditBox());
	empiPanel->add(empiShadowTrailIntervalLabel);
	empiPanel->add(empiShadowTrailInterval);
	empiPanel->add(empiShadowTrailInterval->getEditBox());
	empiPanel->add(empiAnimatableLabel);
	empiPanel->add(empiAnimatable);
	empiPanel->add(empiAnimatable->getAnimatablePicture());
	empiPanel->add(empiLoopAnimationLabel);
	empiPanel->add(empiLoopAnimation);
	empiPanel->add(empiBaseSpriteLabel);
	empiPanel->add(empiBaseSprite);
	empiPanel->add(empiBaseSprite->getAnimatablePicture());
	empiPanel->add(empiDamageLabel);
	empiPanel->add(empiDamage);
	empiPanel->add(empiPierceResetTimeLabel);
	empiPanel->add(empiPierceResetTime);
	empiPanel->add(empiPierceResetTime->getEditBox());
	empiPanel->add(empiSoundSettings);
	empiPanel->add(empiBulletModelLabel);
	empiPanel->add(empiBulletModel);
	empiPanel->add(empiInheritRadiusLabel);
	empiPanel->add(empiInheritRadius);
	empiPanel->add(empiInheritDespawnTimeLabel);
	empiPanel->add(empiInheritDespawnTime);
	empiPanel->add(empiInheritShadowTrailIntervalLabel);
	empiPanel->add(empiInheritShadowTrailInterval);
	empiPanel->add(empiInheritShadowTrailLifespanLabel);
	empiPanel->add(empiInheritShadowTrailLifespan);
	empiPanel->add(empiInheritAnimatablesLabel);
	empiPanel->add(empiInheritAnimatables);
	empiPanel->add(empiInheritDamageLabel);
	empiPanel->add(empiInheritDamage);
	empiPanel->add(empiInheritSoundSettingsLabel);
	empiPanel->add(empiInheritSoundSettings);
	//------------------ Play area window widgets --------------------------------


	//----------------------------------------------------------------------------

	mainWindow->getResizeSignal()->sink().connect<AttackEditor, &AttackEditor::onMainWindowResize>(this);
	onMainWindowResize(mainWindow->getWindowWidth(), mainWindow->getWindowHeight());
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

void AttackEditor::onMainWindowResize(int windowWidth, int windowHeight) {
	const float aiAreaWidth = windowWidth * 0.25f;
	aiPanel->setPosition(0, 0);
	aiID->setMaximumTextWidth(aiAreaWidth - GUI_PADDING_X * 2);
	aiPlayAttackAnimation->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	aiName->setSize(aiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	aiID->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	aiName->setPosition({ tgui::bindLeft(aiID), tgui::bindBottom(aiID) + GUI_PADDING_Y });
	aiPlayAttackAnimation->setPosition({ tgui::bindLeft(aiID), tgui::bindBottom(aiName) + GUI_PADDING_Y });
	aiPanel->setSize(aiAreaWidth, std::min(windowHeight*0.25f, aiPlayAttackAnimation->getPosition().y + aiPlayAttackAnimation->getSize().y + GUI_PADDING_Y));

	const float alAreaWidth = windowWidth * 0.25f;
	const float alAreaHeight = windowHeight - aiPanel->getPosition().y - aiPanel->getSize().y;
	alPanel->setSize(alAreaWidth, alAreaHeight);
	alPanel->setPosition(0, tgui::bindBottom(aiPanel));
	alSaveAll->setSize(100, TEXT_BUTTON_HEIGHT);
	alDiscardAll->setSize(100, TEXT_BUTTON_HEIGHT);
	alCreateAttack->setSize(100, TEXT_BUTTON_HEIGHT);
	alDeleteAttack->setSize(100, TEXT_BUTTON_HEIGHT);
	alList->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	alCreateAttack->setPosition(tgui::bindLeft(alList), alAreaHeight - alCreateAttack->getSize().y - GUI_PADDING_Y);
	alDeleteAttack->setPosition(tgui::bindRight(alCreateAttack) + GUI_PADDING_X, tgui::bindTop(alCreateAttack));
	alSaveAll->setPosition(tgui::bindLeft(alList), tgui::bindTop(alCreateAttack) - alSaveAll->getSize().y - GUI_PADDING_Y);
	alDiscardAll->setPosition(tgui::bindRight(alSaveAll) + GUI_PADDING_X, tgui::bindTop(alSaveAll));
	alList->setSize(alAreaWidth - GUI_PADDING_X * 2, alSaveAll->getPosition().y - GUI_PADDING_Y * 2);

	const float emplAreaWidth = windowWidth * 0.5f;
	emplPanel->setPosition(tgui::bindRight(aiPanel), 0);
	emplPanel->setSize(emplAreaWidth, windowHeight);
	emplLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	emplCreateEMP->setSize(100, TEXT_BUTTON_HEIGHT);
	emplDeleteEMP->setSize(100, TEXT_BUTTON_HEIGHT);
	emplCreateEMP->setPosition(tgui::bindLeft(emplTree), windowHeight - emplCreateEMP->getSize().y - GUI_PADDING_Y);
	emplDeleteEMP->setPosition(tgui::bindRight(emplCreateEMP) + GUI_PADDING_X, tgui::bindTop(emplCreateEMP));
	emplTree->setPosition(tgui::bindLeft(emplLabel), tgui::bindBottom(emplLabel) + GUI_PADDING_Y);
	emplTree->setSize(emplAreaWidth - GUI_PADDING_X * 2, emplCreateEMP->getPosition().y - emplLabel->getPosition().y - emplLabel->getSize().y - GUI_PADDING_Y * 2);


	const float empiAreaWidth = windowWidth - (emplAreaWidth + alAreaWidth);
	empiIsBullet->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	empiHitboxRadius->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiDespawnTime->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiActions->setSize(empiAreaWidth - GUI_PADDING_X * 2, 250);
	empiSpawnType->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiSpawnTypeTime->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiSpawnTypeX->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiSpawnTypeY->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiShadowTrailLifespan->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiShadowTrailInterval->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiAnimatableLabel->setMaximumTextWidth(0);
	auto empiAnimatablePicture = empiAnimatable->getAnimatablePicture();
	empiAnimatablePicture->setSize(100, 100);
	empiAnimatablePicture->setPosition(tgui::bindLeft(empiAnimatableLabel), tgui::bindBottom(empiAnimatableLabel) + GUI_PADDING_Y);
	empiAnimatable->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiAnimatable->setPosition(tgui::bindLeft(empiAnimatableLabel), tgui::bindBottom(empiAnimatablePicture) + GUI_PADDING_Y);
	empiAnimatable->calculateItemsToDisplay();
	empiLoopAnimation->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	auto empiBaseSpritePicture = empiBaseSprite->getAnimatablePicture();
	empiBaseSpritePicture->setSize(100, 100);
	empiBaseSpritePicture->setPosition(tgui::bindLeft(empiBaseSpriteLabel), tgui::bindBottom(empiBaseSpriteLabel) + GUI_PADDING_Y);
	empiBaseSprite->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiBaseSprite->setPosition(tgui::bindLeft(empiBaseSpritePicture), tgui::bindBottom(empiBaseSpritePicture) + GUI_PADDING_Y);
	empiBaseSprite->calculateItemsToDisplay();
	empiDamage->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiOnCollisionAction->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiPierceResetTime->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiSoundSettings->setSize(empiAreaWidth, 0); // height fixed by onContainerResize
	empiSoundSettings->onContainerResize(empiAreaWidth, windowHeight);
	empiBulletModel->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiInheritRadius->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	empiInheritDespawnTime->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	empiInheritShadowTrailInterval->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	empiInheritShadowTrailLifespan->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	empiInheritAnimatables->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	empiInheritDamage->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	empiInheritSoundSettings->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);

	empiPanel->setPosition(tgui::bindRight(emplPanel), 0);
	empiPanel->setSize(empiAreaWidth, windowHeight);
	empiIsBullet->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	empiID->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	empiIsBulletLabel->setPosition(tgui::bindLeft(empiID), tgui::bindBottom(empiID) + GUI_PADDING_Y);
	empiIsBullet->setPosition(tgui::bindLeft(empiIsBulletLabel), tgui::bindBottom(empiIsBulletLabel) + GUI_PADDING_Y);
	empiHitboxRadiusLabel->setPosition(tgui::bindLeft(empiIsBullet), tgui::bindBottom(empiIsBullet) + GUI_PADDING_Y);
	empiHitboxRadius->setPosition(empiHitboxRadiusLabel->getPosition().x, empiHitboxRadiusLabel->getPosition().y + empiHitboxRadiusLabel->getSize().y + GUI_PADDING_Y);
	empiDespawnTimeLabel->setPosition(tgui::bindLeft(empiHitboxRadius), tgui::bindBottom(empiHitboxRadius) + GUI_PADDING_Y);
	empiDespawnTime->setPosition(empiDespawnTimeLabel->getPosition().x, empiDespawnTimeLabel->getPosition().y + empiDespawnTimeLabel->getSize().y + GUI_PADDING_Y);
	empiActionsLabel->setPosition(tgui::bindLeft(empiDespawnTime), tgui::bindBottom(empiDespawnTime) + GUI_PADDING_Y);
	empiActions->setPosition(tgui::bindLeft(empiActionsLabel), tgui::bindBottom(empiActionsLabel) + GUI_PADDING_Y);
	empiSpawnTypeLabel->setPosition(tgui::bindLeft(empiActions), tgui::bindBottom(empiActions) + GUI_PADDING_Y);
	empiSpawnType->setPosition(tgui::bindLeft(empiSpawnTypeLabel), tgui::bindBottom(empiSpawnTypeLabel) + GUI_PADDING_Y);
	empiSpawnTypeTimeLabel->setPosition(tgui::bindLeft(empiSpawnType), tgui::bindBottom(empiSpawnType) + GUI_PADDING_Y);
	empiSpawnTypeTime->setPosition(tgui::bindLeft(empiSpawnTypeTimeLabel), tgui::bindBottom(empiSpawnTypeTimeLabel) + GUI_PADDING_Y);
	empiSpawnTypeXLabel->setPosition(tgui::bindLeft(empiSpawnTypeTime), tgui::bindBottom(empiSpawnTypeTime) + GUI_PADDING_Y);
	empiSpawnTypeX->setPosition(tgui::bindLeft(empiSpawnTypeXLabel), tgui::bindBottom(empiSpawnTypeXLabel) + GUI_PADDING_Y);
	empiSpawnTypeYLabel->setPosition(tgui::bindLeft(empiSpawnTypeX), tgui::bindBottom(empiSpawnTypeX) + GUI_PADDING_Y);
	empiSpawnTypeY->setPosition(tgui::bindLeft(empiSpawnTypeYLabel), tgui::bindBottom(empiSpawnTypeYLabel) + GUI_PADDING_Y);
	empiShadowTrailLifespanLabel->setPosition(tgui::bindLeft(empiSpawnTypeY), tgui::bindBottom(empiSpawnTypeY) + GUI_PADDING_Y);
	empiShadowTrailLifespan->setPosition(empiShadowTrailLifespanLabel->getPosition().x, empiShadowTrailLifespanLabel->getPosition().y + empiShadowTrailLifespanLabel->getSize().y + GUI_PADDING_Y);
	empiShadowTrailIntervalLabel->setPosition(tgui::bindLeft(empiShadowTrailLifespan), tgui::bindBottom(empiShadowTrailLifespan) + GUI_PADDING_Y);
	empiShadowTrailInterval->setPosition(empiShadowTrailIntervalLabel->getPosition().x, empiShadowTrailIntervalLabel->getPosition().y + empiShadowTrailIntervalLabel->getSize().y + GUI_PADDING_Y);
	empiAnimatableLabel->setPosition(tgui::bindLeft(empiShadowTrailInterval), tgui::bindBottom(empiShadowTrailInterval) + GUI_PADDING_Y);
	//empiAnimatable->setPosition(tgui::bindLeft(empiAnimatableLabel), tgui::bindBottom(empiAnimatableLabel) + GUI_PADDING_Y);
	empiLoopAnimationLabel->setPosition(tgui::bindLeft(empiAnimatable), tgui::bindBottom(empiAnimatable) + GUI_PADDING_Y);
	empiLoopAnimation->setPosition(tgui::bindLeft(empiLoopAnimationLabel), tgui::bindBottom(empiLoopAnimationLabel) + GUI_PADDING_Y);
	empiBaseSpriteLabel->setPosition(tgui::bindLeft(empiLoopAnimation), tgui::bindBottom(empiLoopAnimation) + GUI_PADDING_Y);
	//empiBaseSprite->setPosition(tgui::bindLeft(empiBaseSpriteLabel), tgui::bindBottom(empiBaseSpriteLabel) + GUI_PADDING_Y);
	empiDamageLabel->setPosition(tgui::bindLeft(empiBaseSprite), tgui::bindBottom(empiBaseSprite) + GUI_PADDING_Y);
	empiDamage->setPosition(tgui::bindLeft(empiDamageLabel), tgui::bindBottom(empiDamageLabel) + GUI_PADDING_Y);
	empiOnCollisionActionLabel->setPosition(tgui::bindLeft(empiDamage), tgui::bindBottom(empiDamage) + GUI_PADDING_Y);
	empiOnCollisionAction->setPosition(tgui::bindLeft(empiOnCollisionActionLabel), tgui::bindBottom(empiOnCollisionActionLabel) + GUI_PADDING_Y);
	empiPierceResetTimeLabel->setPosition(tgui::bindLeft(empiOnCollisionAction), tgui::bindBottom(empiOnCollisionAction) + GUI_PADDING_Y);
	empiPierceResetTime->setPosition(empiPierceResetTimeLabel->getPosition().x, empiPierceResetTimeLabel->getPosition().y + empiPierceResetTimeLabel->getSize().y + GUI_PADDING_Y);
	empiSoundSettings->setPosition(0, tgui::bindBottom(empiPierceResetTime) + GUI_PADDING_Y);
	empiBulletModelLabel->setPosition(tgui::bindLeft(empiPierceResetTime), tgui::bindBottom(empiSoundSettings) + GUI_PADDING_Y);
	empiBulletModel->setPosition(tgui::bindLeft(empiBulletModelLabel), tgui::bindBottom(empiBulletModelLabel) + GUI_PADDING_Y);
	empiInheritRadiusLabel->setPosition(tgui::bindLeft(empiBulletModel), tgui::bindBottom(empiBulletModel) + GUI_PADDING_Y);
	empiInheritRadius->setPosition(tgui::bindLeft(empiInheritRadiusLabel), tgui::bindBottom(empiInheritRadiusLabel) + GUI_PADDING_Y);
	empiInheritDespawnTimeLabel->setPosition(tgui::bindLeft(empiInheritRadius), tgui::bindBottom(empiInheritRadius) + GUI_PADDING_Y);
	empiInheritDespawnTime->setPosition(tgui::bindLeft(empiInheritDespawnTimeLabel), tgui::bindBottom(empiInheritDespawnTimeLabel) + GUI_PADDING_Y);
	empiInheritShadowTrailIntervalLabel->setPosition(tgui::bindLeft(empiInheritDespawnTime), tgui::bindBottom(empiInheritDespawnTime) + GUI_PADDING_Y);
	empiInheritShadowTrailInterval->setPosition(tgui::bindLeft(empiInheritShadowTrailIntervalLabel), tgui::bindBottom(empiInheritShadowTrailIntervalLabel) + GUI_PADDING_Y);
	empiInheritShadowTrailLifespanLabel->setPosition(tgui::bindLeft(empiInheritShadowTrailInterval), tgui::bindBottom(empiInheritShadowTrailInterval) + GUI_PADDING_Y);
	empiInheritShadowTrailLifespan->setPosition(tgui::bindLeft(empiInheritShadowTrailLifespanLabel), tgui::bindBottom(empiInheritShadowTrailLifespanLabel) + GUI_PADDING_Y);
	empiInheritAnimatablesLabel->setPosition(tgui::bindLeft(empiInheritShadowTrailLifespan), tgui::bindBottom(empiInheritShadowTrailLifespan) + GUI_PADDING_Y);
	empiInheritAnimatables->setPosition(tgui::bindLeft(empiInheritAnimatablesLabel), tgui::bindBottom(empiInheritAnimatablesLabel) + GUI_PADDING_Y);
	empiInheritDamageLabel->setPosition(tgui::bindLeft(empiInheritAnimatables), tgui::bindBottom(empiInheritAnimatables) + GUI_PADDING_Y);
	empiInheritDamage->setPosition(tgui::bindLeft(empiInheritDamageLabel), tgui::bindBottom(empiInheritDamageLabel) + GUI_PADDING_Y);
	empiInheritSoundSettingsLabel->setPosition(tgui::bindLeft(empiInheritDamage), tgui::bindBottom(empiInheritDamage) + GUI_PADDING_Y);
	empiInheritSoundSettings->setPosition(tgui::bindLeft(empiInheritSoundSettingsLabel), tgui::bindBottom(empiInheritSoundSettingsLabel) + GUI_PADDING_Y);
}

void AttackEditor::selectAttack(int id) {
	if (unsavedAttacks.count(id) > 0) {
		selectedAttack = unsavedAttacks[id];
	} else {
		// Make a copy of the attack in the LevelPack so that changes can be applied/discarded
		// whenever the user wants instead of modifying the LevelPack directly.
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
	// A copy of the EMP is not necessary because a copy of the selected attack has already been made
	// when selecting an attack.
	selectedEMP = selectedAttack->searchEMP(empID);

	emplCreateEMP->setEnabled(true);
	// Can't delete main EMP of an attack
	emplDeleteEMP->setEnabled(empID != selectedAttack->getMainEMP()->getID());

	bool isBullet = selectedEMP->getIsBullet();
	bool usesModel = selectedEMP->usesBulletModel();
	empiID->setVisible(true);
	empiIsBulletLabel->setVisible(true);
	empiIsBullet->setVisible(true);
	empiHitboxRadiusLabel->setVisible(isBullet);
	empiHitboxRadius->setVisible(isBullet);
	empiDespawnTimeLabel->setVisible(true);
	empiDespawnTime->setVisible(true);
	empiActionsLabel->setVisible(true);
	empiActions->setVisible(true);
	empiSpawnTypeLabel->setVisible(true);
	empiSpawnType->setVisible(true);
	empiSpawnTypeTimeLabel->setVisible(true);
	empiSpawnTypeTime->setVisible(true);
	empiSpawnTypeXLabel->setVisible(true);
	empiSpawnTypeX->setVisible(true);
	empiSpawnTypeYLabel->setVisible(true);
	empiSpawnTypeY->setVisible(true);
	empiShadowTrailLifespanLabel->setVisible(isBullet);
	empiShadowTrailLifespan->setVisible(isBullet);
	empiShadowTrailIntervalLabel->setVisible(isBullet);
	empiShadowTrailInterval->setVisible(isBullet);
	empiAnimatableLabel->setVisible(isBullet);
	empiAnimatable->setVisible(isBullet);
	empiLoopAnimationLabel->setVisible(isBullet);
	empiLoopAnimation->setVisible(isBullet);
	empiBaseSpriteLabel->setVisible(isBullet);
	empiBaseSprite->setVisible(isBullet);
	empiDamageLabel->setVisible(isBullet);
	empiDamage->setVisible(isBullet);
	empiOnCollisionActionLabel->setVisible(isBullet);
	empiOnCollisionAction->setVisible(isBullet);
	empiSoundSettings->setVisible(true);
	empiBulletModelLabel->setVisible(isBullet);
	empiBulletModel->setVisible(isBullet);
	empiInheritRadiusLabel->setVisible(isBullet && usesModel);
	empiInheritRadius->setVisible(isBullet && usesModel);
	empiInheritDespawnTimeLabel->setVisible(isBullet && usesModel);
	empiInheritDespawnTime->setVisible(isBullet && usesModel);
	empiInheritShadowTrailIntervalLabel->setVisible(isBullet && usesModel);
	empiInheritShadowTrailInterval->setVisible(isBullet && usesModel);
	empiInheritShadowTrailLifespanLabel->setVisible(isBullet && usesModel);
	empiInheritShadowTrailLifespan->setVisible(isBullet && usesModel);
	empiInheritAnimatablesLabel->setVisible(isBullet && usesModel);
	empiInheritAnimatables->setVisible(isBullet && usesModel);
	empiInheritDamageLabel->setVisible(isBullet && usesModel);
	empiInheritDamage->setVisible(isBullet && usesModel);
	empiInheritSoundSettingsLabel->setVisible(isBullet && usesModel);
	empiInheritSoundSettings->setVisible(isBullet && usesModel);

	empiID->setText("ID: " + std::to_string(selectedEMP->getID()));
	empiIsBullet->setChecked(selectedEMP->getIsBullet());
	empiSpawnType->setSelectedItemById(getID(selectedEMP->getSpawnType()));
	empiSpawnTypeTime->setValue(selectedEMP->getSpawnType()->getTime());
	empiSpawnTypeX->setValue(selectedEMP->getSpawnType()->getX());
	empiSpawnTypeY->setValue(selectedEMP->getSpawnType()->getY());
	empiShadowTrailLifespan->setValue(selectedEMP->getShadowTrailLifespan());
	empiOnCollisionAction->setSelectedItemById(getID(selectedEMP->getOnCollisionAction()));
	empiPierceResetTime->setValue(selectedEMP->getPierceResetTime());
	buildEMPIActions();

	if (usesModel) {
		empiBulletModel->setSelectedItemById(std::to_string(selectedEMP->getBulletModelID()));

		std::shared_ptr<BulletModel> model = levelPack.getBulletModel(selectedEMP->getBulletModelID());
		if (selectedEMP->getInheritAnimatables()) {
			empiAnimatable->setSelectedItem(model->getAnimatable());
			empiLoopAnimation->setChecked(model->getLoopAnimation());
			empiBaseSprite->setSelectedItem(model->getBaseSprite());
		} else {
			empiAnimatable->setSelectedItem(selectedEMP->getAnimatable());
			empiLoopAnimation->setChecked(selectedEMP->getLoopAnimation());
			empiBaseSprite->setSelectedItem(selectedEMP->getBaseSprite());
		}

		if (selectedEMP->getInheritDamage()) {
			empiDamage->setValue(model->getDamage());
		} else {
			empiDamage->setValue(selectedEMP->getDamage());
		}

		if (selectedEMP->getInheritDespawnTime()) {
			empiDespawnTime->setValue(model->getDespawnTime());
		} else {
			empiDespawnTime->setValue(selectedEMP->getDespawnTime());
		}

		if (selectedEMP->getInheritRadius()) {
			empiHitboxRadius->setValue(model->getHitboxRadius());
		} else {
			empiHitboxRadius->setValue(selectedEMP->getHitboxRadius());
		}

		if (selectedEMP->getInheritShadowTrailInterval()) {
			empiShadowTrailInterval->setValue(model->getShadowTrailInterval());
		} else {
			empiShadowTrailInterval->setValue(selectedEMP->getShadowTrailInterval());
		}

		if (selectedEMP->getInheritShadowTrailLifespan()) {
			empiShadowTrailLifespan->setValue(model->getShadowTrailLifespan());
		} else {
			empiShadowTrailInterval->setValue(selectedEMP->getShadowTrailLifespan());
		}

		if (selectedEMP->getInheritSoundSettings()) {
			empiSoundSettings->initSettings(model->getSoundSettings());
		} else {
			empiSoundSettings->initSettings(selectedEMP->getSoundSettings());
		}
	} else {
		empiAnimatable->setSelectedItem(selectedEMP->getAnimatable());
		empiLoopAnimation->setChecked(selectedEMP->getLoopAnimation());
		empiBaseSprite->setSelectedItem(selectedEMP->getBaseSprite());
		empiDamage->setValue(selectedEMP->getDamage());
		empiDespawnTime->setValue(selectedEMP->getDespawnTime());
		empiHitboxRadius->setValue(selectedEMP->getHitboxRadius());
		empiShadowTrailInterval->setValue(selectedEMP->getShadowTrailInterval());
		empiShadowTrailInterval->setValue(selectedEMP->getShadowTrailLifespan());
		empiSoundSettings->initSettings(selectedEMP->getSoundSettings());
	}
	empiAnimatable->setEnabled(!usesModel || !selectedEMP->getInheritAnimatables());
	empiLoopAnimation->setEnabled(!usesModel || !selectedEMP->getInheritAnimatables());
	empiBaseSprite->setEnabled(!usesModel || !selectedEMP->getInheritAnimatables());
	empiDamage->setEnabled(!usesModel || !selectedEMP->getInheritDamage());
	empiDespawnTime->setEnabled(!usesModel || !selectedEMP->getInheritDespawnTime());
	empiHitboxRadius->setEnabled(!usesModel || !selectedEMP->getInheritRadius());
	empiShadowTrailInterval->setEnabled(!usesModel || !selectedEMP->getInheritShadowTrailInterval());
	empiShadowTrailLifespan->setEnabled(!usesModel || !selectedEMP->getInheritShadowTrailLifespan());
	empiSoundSettings->setEnabled(!usesModel || !selectedEMP->getInheritSoundSettings());
}

void AttackEditor::deselectEMP() {
	selectedEMP = nullptr;

	emplCreateEMP->setEnabled(false);
	emplDeleteEMP->setEnabled(false);

	empiID->setVisible(false);
	empiIsBulletLabel->setVisible(false);
	empiIsBullet->setVisible(false);
	empiHitboxRadiusLabel->setVisible(false);
	empiHitboxRadius->setVisible(false);
	empiDespawnTimeLabel->setVisible(false);
	empiDespawnTime->setVisible(false);
	empiActionsLabel->setVisible(false);
	empiActions->setVisible(false);
	empiSpawnTypeLabel->setVisible(false);
	empiSpawnType->setVisible(false);
	empiSpawnTypeTimeLabel->setVisible(false);
	empiSpawnTypeTime->setVisible(false);
	empiSpawnTypeXLabel->setVisible(false);
	empiSpawnTypeX->setVisible(false);
	empiSpawnTypeYLabel->setVisible(false);
	empiSpawnTypeY->setVisible(false);
	empiShadowTrailLifespanLabel->setVisible(false);
	empiShadowTrailLifespan->setVisible(false);
	empiShadowTrailIntervalLabel->setVisible(false);
	empiShadowTrailInterval->setVisible(false);
	empiAnimatableLabel->setVisible(false);
	empiAnimatable->setVisible(false);
	empiLoopAnimationLabel->setVisible(false);
	empiLoopAnimation->setVisible(false);
	empiBaseSpriteLabel->setVisible(false);
	empiBaseSprite->setVisible(false);
	empiDamageLabel->setVisible(false);
	empiDamage->setVisible(false);
	empiOnCollisionActionLabel->setVisible(false);
	empiOnCollisionAction->setVisible(false);
	empiPierceResetTimeLabel->setVisible(false);
	empiPierceResetTime->setVisible(false);
	empiSoundSettings->setVisible(false);
	empiBulletModelLabel->setVisible(false);
	empiBulletModel->setVisible(false);
	empiInheritRadiusLabel->setVisible(false);
	empiInheritRadius->setVisible(false);
	empiInheritDespawnTimeLabel->setVisible(false);
	empiInheritDespawnTime->setVisible(false);
	empiInheritShadowTrailIntervalLabel->setVisible(false);
	empiInheritShadowTrailInterval->setVisible(false);
	empiInheritShadowTrailLifespanLabel->setVisible(false);
	empiInheritShadowTrailLifespan->setVisible(false);
	empiInheritAnimatablesLabel->setVisible(false);
	empiInheritAnimatables->setVisible(false);
	empiInheritDamageLabel->setVisible(false);
	empiInheritDamage->setVisible(false);
	empiInheritSoundSettingsLabel->setVisible(false);
	empiInheritSoundSettings->setVisible(false);
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
	std::string bulletStr = emp.getIsBullet() ? "[B]" : "[A]";
	std::string idStr = "[id=" + std::to_string(emp.getID()) + "]";
	return bulletStr + " " + idStr;
}

void AttackEditor::buildEMPIActions() {
	empiActions->removeAllItems();
	auto actions = selectedEMP->getActions();
	float curTime = 0;
	for (int i = 0; i < actions.size(); i++) {
		empiActions->addItem("[t=" + formatNum(curTime) + "] " + actions[i]->getGuiFormat(), std::to_string(i));
	}
}

void AttackEditor::onMainWindowRender(float deltaTime) {
	std::lock_guard<std::mutex> lock(*tguiMutex);

	empiAnimatable->getAnimatablePicture()->update(deltaTime);
	//TODO: add the other animatable stuff
}
