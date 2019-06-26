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

AttackEditor::AttackEditor(LevelPack& levelPack, std::shared_ptr<SpriteLoader> spriteLoader) : levelPack(levelPack), spriteLoader(spriteLoader), mainWindowUndoStack(UndoStack(UNDO_STACK_MAX)), playAreaWindowUndoStack(UndoStack(UNDO_STACK_MAX)) {
	tguiMutex = std::make_shared<std::mutex>();

	mainWindow = std::make_shared<UndoableEditorWindow>(tguiMutex, "Attack Editor", MAIN_WINDOW_WIDTH, 400, mainWindowUndoStack);
	playAreaWindow = std::make_shared<UndoableEditorWindow>(tguiMutex, "Attack Editor - Gameplay Test", MAP_WIDTH, MAP_HEIGHT, playAreaWindowUndoStack);

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

	aiName->connect("ReturnKeyPressed", [this, &selectedAttack = this->selectedAttack](std::string text) {
		if (selectedAttack && text != selectedAttack->getName()) {
			std::string oldName = selectedAttack->getName();
			mainWindowUndoStack.execute(UndoableCommand(
				[this, selectedAttack, text]() {
				selectedAttack->setName(text);
				setAttackWidgetValues(selectedAttack, true, true);
			},
				[this, selectedAttack, oldName]() {
				selectedAttack->setName(oldName);
				setAttackWidgetValues(selectedAttack, true, true);
			}));
		}
	});
	aiPlayAttackAnimation->connect("Changed", [this, &selectedAttack = this->selectedAttack]() {
		bool checked = aiPlayAttackAnimation->isChecked();
		if (selectedAttack && selectedAttack->getPlayAttackAnimation() != checked) {
			std::string oldName = selectedAttack->getName();
			mainWindowUndoStack.execute(UndoableCommand(
				[this, selectedAttack, checked]() {
				selectedAttack->setPlayAttackAnimation(checked);
				setAttackWidgetValues(selectedAttack, true, true);
			},
				[this, selectedAttack, checked]() {
				selectedAttack->setPlayAttackAnimation(!checked);
				setAttackWidgetValues(selectedAttack, true, true);
			}));
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
	alCreateAttack->setTextSize(TEXT_SIZE);
	alDeleteAttack->setTextSize(TEXT_SIZE);

	alSaveAll->setText("Save all");
	alDiscardAll->setText("Discard all");
	alCreateAttack->setText("New");
	alDeleteAttack->setText("Delete");
	alDeleteAttack->setEnabled(false);

	buildAttackList(false);

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
	alCreateAttack->connect("Pressed", [this, &levelPack = this->levelPack]() {
		std::shared_ptr<EditorAttack> newAttack = levelPack.createAttack(false);
		mainWindowUndoStack.execute(UndoableCommand(
			[this, &levelPack, newAttack]() {
			levelPack.updateAttack(newAttack);
			buildAttackList(true);
		},
			[this, &levelPack, newAttack]() {
			deleteAttack(newAttack, true);
		}));
	});
	alDeleteAttack->connect("Pressed", [this, &levelPack = this->levelPack, &selectedAttack = this->selectedAttack]() {
		mainWindowUndoStack.execute(UndoableCommand(
			[this, selectedAttack]() {
			deleteAttack(selectedAttack);
		},
			[this, &levelPack, selectedAttack]() {
			levelPack.updateAttack(selectedAttack);
			buildAttackList(false);
		}));
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

	emplCreateEMP->setTextSize(TEXT_SIZE);
	emplDeleteEMP->setTextSize(TEXT_SIZE);
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
	empiActionsAddAbove = tgui::Button::create();
	empiActionsAddBelow = tgui::Button::create();
	empiActionsDelete = tgui::Button::create();
	empiSpawnTypeLabel = tgui::Label::create();
	empiSpawnType = tgui::ComboBox::create();
	empiSpawnTypeTimeLabel = tgui::Label::create();
	empiSpawnTypeTime = std::make_shared<NumericalEditBoxWithLimits>();
	empiSpawnTypeXLabel = tgui::Label::create();
	empiSpawnTypeX = std::make_shared<NumericalEditBoxWithLimits>();
	empiSpawnTypeYLabel = tgui::Label::create();
	empiSpawnTypeY = std::make_shared<NumericalEditBoxWithLimits>();
	empiSpawnLocationManualSet = tgui::Button::create();
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
	empiActionsAddAbove->setTextSize(TEXT_SIZE);
	empiActionsAddBelow->setTextSize(TEXT_SIZE);
	empiActionsDelete->setTextSize(TEXT_SIZE);
	empiSpawnTypeLabel->setTextSize(TEXT_SIZE);
	empiSpawnType->setTextSize(TEXT_SIZE);
	empiSpawnTypeTimeLabel->setTextSize(TEXT_SIZE);
	empiSpawnTypeTime->setTextSize(TEXT_SIZE);
	empiSpawnTypeXLabel->setTextSize(TEXT_SIZE);
	empiSpawnTypeX->setTextSize(TEXT_SIZE);
	empiSpawnTypeYLabel->setTextSize(TEXT_SIZE);
	empiSpawnTypeY->setTextSize(TEXT_SIZE);
	empiSpawnLocationManualSet->setTextSize(TEXT_SIZE);
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
	empiActionsAddAbove->setText("Add above");
	empiActionsAddBelow->setText("Add below");
	empiActionsDelete->setText("Delete");
	empiSpawnTypeLabel->setText("Spawn type");
	empiSpawnTypeTimeLabel->setText("Spawn delay");
	empiSpawnTypeXLabel->setText("Spawn X");
	empiSpawnTypeYLabel->setText("Spawn Y");
	empiSpawnLocationManualSet->setText("Manual set");
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

	empiIsBullet->connect("Changed", [this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack]() {
		bool checked = empiIsBullet->isChecked();
		if (checked != selectedEMP->getIsBullet() && !skipUndoCommandCreation) {
			mainWindowUndoStack.execute(UndoableCommand(
				[this, selectedEMP, selectedAttack, checked]() {
				selectedEMP->setIsBullet(checked);
				setEMPWidgetValues(selectedEMP, selectedAttack, true, false);
			},
				[this, selectedEMP, selectedAttack, checked]() {
				selectedEMP->setIsBullet(!checked);
				setEMPWidgetValues(selectedEMP, selectedAttack, true, false);
			}));
		}
	});
	empiHitboxRadius->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onEmpiHitboxRadiusChange>(this);
	empiDespawnTime->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onEmpiDespawnTimeChange>(this);
	empiActions->connect("ItemSelected", [&](std::string item, std::string id) {
		if (id == "") {
			deselectEMPA();
		} else {
			selectEMPA(std::stoi(id));
		}
	});
	empiActionsAddAbove->connect("Pressed", [&]() {
		auto popup = tgui::ListBox::create();
		popup->addItem("Bezier movement", "1");
		popup->addItem("Polar movement", "2");
		popup->addItem("Homing movement", "3");
		popup->addItem("Stay still", "4");
		popup->addItem("Detach", "5");
		popup->setItemHeight(20);
		popup->setPosition(empiActionsAddAbove->getPosition());
		popup->setSize(150, popup->getItemHeight() * popup->getItemCount());
		popup->connect("ItemSelected", [&](std::string item, std::string id) {
			int index;
			if (selectedEMPAIndex == -1) {
				index = 0;
			} else {
				index = selectedEMPAIndex;
			}
			if (id == "1") {
				selectedEMP->insertAction(index, std::make_shared<MoveCustomBezierEMPA>());
			} else if (id == "2") {
				selectedEMP->insertAction(index, std::make_shared<MoveCustomPolarEMPA>());
			} else if (id == "3") {
				selectedEMP->insertAction(index, std::make_shared<MovePlayerHomingEMPA>());
			} else if (id == "4") {
				selectedEMP->insertAction(index, std::make_shared<StayStillAtLastPositionEMPA>());
			} else if (id == "5") {
				selectedEMP->insertAction(index, std::make_shared<DetachFromParentEMPA>());
			}
			buildEMPIActions();
		});
		mainWindow->addPopupWidget(empiPanel, popup);
	});
	empiActionsAddBelow->connect("Pressed", [&]() {
		auto popup = tgui::ListBox::create();
		popup->addItem("Bezier movement", "1");
		popup->addItem("Polar movement", "2");
		popup->addItem("Homing movement", "3");
		popup->addItem("Stay still", "4");
		popup->addItem("Detach", "5");
		popup->setItemHeight(20);
		popup->setPosition(empiActionsAddBelow->getPosition());
		popup->setSize(150, popup->getItemHeight() * popup->getItemCount());
		popup->connect("ItemSelected", [&](std::string item, std::string id) {
			int index;
			if (selectedEMPAIndex == -1) {
				index = selectedEMP->getActions().size();
			} else {
				index = selectedEMPAIndex + 1;
			}
			if (id == "1") {
				selectedEMP->insertAction(index, std::make_shared<MoveCustomBezierEMPA>());
			} else if (id == "2") {
				selectedEMP->insertAction(index, std::make_shared<MoveCustomPolarEMPA>());
			} else if (id == "3") {
				selectedEMP->insertAction(index, std::make_shared<MovePlayerHomingEMPA>());
			} else if (id == "4") {
				selectedEMP->insertAction(index, std::make_shared<StayStillAtLastPositionEMPA>());
			} else if (id == "5") {
				selectedEMP->insertAction(index, std::make_shared<DetachFromParentEMPA>());
			}
			buildEMPIActions();
		});
		mainWindow->addPopupWidget(empiPanel, popup);
	});
	
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
	empiPanel->add(empiActionsAddAbove);
	empiPanel->add(empiActionsAddBelow);
	empiPanel->add(empiActionsDelete);
	empiPanel->add(empiSpawnTypeLabel);
	empiPanel->add(empiSpawnType);
	empiPanel->add(empiSpawnTypeTimeLabel);
	empiPanel->add(empiSpawnTypeTime);
	empiPanel->add(empiSpawnTypeXLabel);
	empiPanel->add(empiSpawnTypeX);
	empiPanel->add(empiSpawnTypeYLabel);
	empiPanel->add(empiSpawnTypeY);
	empiPanel->add(empiSpawnLocationManualSet);
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


	//------------------ EMPA info widgets ---------------------------------------
	empaiPanel = tgui::ScrollablePanel::create();
	empaiInfo = tgui::Label::create();
	empaiDurationLabel = tgui::Label::create();
	empaiDuration = std::make_shared<SliderWithEditBox>();
	empaiPolarDistanceLabel = tgui::Label::create();
	empaiPolarDistance = std::make_shared<TFVGroup>();
	empaiPolarAngleLabel = tgui::Label::create();
	empaiPolarAngle = std::make_shared<TFVGroup>();
	empaiBezierControlPointsLabel = tgui::Label::create();
	empaiBezierControlPoints = tgui::ListBox::create();
	empaiAngleOffsetLabel = tgui::Label::create();
	empaiAngleOffset = std::make_shared<EMPAAngleOffsetGroup>();
	empaiHomingStrengthLabel = tgui::Label::create();
	empaiHomingStrength = std::make_shared<TFVGroup>();
	empaiHomingSpeedLabel = tgui::Label::create();
	empaiHomingSpeed = std::make_shared<TFVGroup>();

	empaiInfo->setTextSize(TEXT_SIZE);
	empaiDurationLabel->setTextSize(TEXT_SIZE);
	empaiPolarDistanceLabel->setTextSize(TEXT_SIZE);
	empaiPolarAngleLabel->setTextSize(TEXT_SIZE);
	empaiBezierControlPointsLabel->setTextSize(TEXT_SIZE);
	empaiAngleOffsetLabel->setTextSize(TEXT_SIZE);
	empaiHomingStrengthLabel->setTextSize(TEXT_SIZE);
	empaiHomingSpeedLabel->setTextSize(TEXT_SIZE);

	empaiDurationLabel->setText("Duration");
	empaiPolarDistanceLabel->setText("Distance as function of time");
	empaiPolarAngleLabel->setText("Distance as function of time");
	empaiBezierControlPointsLabel->setText("Bezier control points");
	empaiAngleOffsetLabel->setText("Angle offset");
	empaiHomingStrengthLabel->setText("Homing strength as function of time");
	empaiHomingSpeedLabel->setText("Speed as function of time");

	empaiPanel->add(empaiInfo);
	empaiPanel->add(empaiDurationLabel);
	empaiPanel->add(empaiDuration);
	empaiPanel->add(empaiDuration->getEditBox());
	empaiPanel->add(empaiPolarDistanceLabel);
	empaiPanel->add(empaiPolarDistance);
	empaiPanel->add(empaiPolarAngleLabel);
	empaiPanel->add(empaiPolarAngle);
	empaiPanel->add(empaiBezierControlPointsLabel);
	empaiPanel->add(empaiBezierControlPoints);
	empaiPanel->add(empaiAngleOffsetLabel);
	empaiPanel->add(empaiAngleOffset);
	empaiPanel->add(empaiHomingStrengthLabel);
	empaiPanel->add(empaiHomingStrength);
	empaiPanel->add(empaiHomingSpeedLabel);
	empaiPanel->add(empaiHomingSpeed);
	//----------------------------------------------------------------------------

	mainWindow->getResizeSignal()->sink().connect<AttackEditor, &AttackEditor::onMainWindowResize>(this);
	onMainWindowResize(mainWindow->getWindowWidth(), mainWindow->getWindowHeight());

	playAreaWindow->getResizeSignal()->sink().connect<AttackEditor, &AttackEditor::onPlayAreaWindowResize>(this);
	onPlayAreaWindowResize(playAreaWindow->getWindowWidth(), playAreaWindow->getWindowHeight());
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

	std::shared_ptr<tgui::Gui> playAreaGui = playAreaWindow->getGui();
	playAreaGui->add(empaiPanel);

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

	const float emplAreaWidth = windowWidth * 0.375f;
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
	empiActionsAddAbove->setSize(100, TEXT_BOX_HEIGHT);
	empiActionsAddBelow->setSize(100, TEXT_BOX_HEIGHT);
	empiActionsDelete->setSize(100, TEXT_BOX_HEIGHT);
	empiSpawnType->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiSpawnTypeTime->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiSpawnTypeX->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiSpawnTypeY->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiSpawnLocationManualSet->setSize(100, TEXT_BOX_HEIGHT);
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
	empiActionsAddAbove->setPosition(tgui::bindLeft(empiActions), tgui::bindBottom(empiActions) + GUI_PADDING_Y);
	empiActionsAddBelow->setPosition(tgui::bindRight(empiActionsAddAbove) + GUI_PADDING_X, tgui::bindTop(empiActionsAddAbove));
	empiActionsDelete->setPosition(tgui::bindLeft(empiActions), tgui::bindBottom(empiActionsAddAbove) + GUI_PADDING_Y);
	empiSpawnTypeLabel->setPosition(tgui::bindLeft(empiActions), tgui::bindBottom(empiActionsDelete) + GUI_PADDING_Y);
	empiSpawnType->setPosition(tgui::bindLeft(empiSpawnTypeLabel), tgui::bindBottom(empiSpawnTypeLabel) + GUI_PADDING_Y);
	empiSpawnTypeTimeLabel->setPosition(tgui::bindLeft(empiSpawnType), tgui::bindBottom(empiSpawnType) + GUI_PADDING_Y);
	empiSpawnTypeTime->setPosition(tgui::bindLeft(empiSpawnTypeTimeLabel), tgui::bindBottom(empiSpawnTypeTimeLabel) + GUI_PADDING_Y);
	empiSpawnTypeXLabel->setPosition(tgui::bindLeft(empiSpawnTypeTime), tgui::bindBottom(empiSpawnTypeTime) + GUI_PADDING_Y);
	empiSpawnTypeX->setPosition(tgui::bindLeft(empiSpawnTypeXLabel), tgui::bindBottom(empiSpawnTypeXLabel) + GUI_PADDING_Y);
	empiSpawnTypeYLabel->setPosition(tgui::bindLeft(empiSpawnTypeX), tgui::bindBottom(empiSpawnTypeX) + GUI_PADDING_Y);
	empiSpawnTypeY->setPosition(tgui::bindLeft(empiSpawnTypeYLabel), tgui::bindBottom(empiSpawnTypeYLabel) + GUI_PADDING_Y);
	empiSpawnLocationManualSet->setPosition(tgui::bindLeft(empiSpawnTypeY), tgui::bindBottom(empiSpawnTypeY) + GUI_PADDING_Y);
	empiShadowTrailLifespanLabel->setPosition(tgui::bindLeft(empiSpawnLocationManualSet), tgui::bindBottom(empiSpawnLocationManualSet) + GUI_PADDING_Y*2);
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

void AttackEditor::onPlayAreaWindowResize(int windowWidth, int windowHeight) {
	const float empaiAreaWidth = windowWidth * 0.33f;
	empaiPanel->setPosition(windowWidth - empaiAreaWidth, 0);
	empaiPanel->setSize(empaiAreaWidth, windowHeight);
	empaiDuration->setSize(empaiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empaiPolarDistance->setSize(windowWidth, 0);
	empaiPolarDistance->onContainerResize(empaiAreaWidth, windowHeight);
	empaiPolarAngle->setSize(windowWidth, 0);
	empaiPolarAngle->onContainerResize(empaiAreaWidth, windowHeight);
	empaiBezierControlPoints->setSize(empaiAreaWidth - GUI_PADDING_X * 2, 200);
	empaiAngleOffset->setSize(windowWidth, 0);
	empaiAngleOffset->onContainerResize(empaiAreaWidth, windowHeight);
	empaiHomingStrength->setSize(windowWidth, 0);
	empaiHomingStrength->onContainerResize(empaiAreaWidth, windowHeight);
	empaiHomingSpeed->setSize(windowWidth, 0);
	empaiHomingSpeed->onContainerResize(empaiAreaWidth, windowHeight);

	empaiInfo->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	empaiDurationLabel->setPosition(tgui::bindLeft(empaiInfo), tgui::bindBottom(empaiInfo) + GUI_PADDING_Y);
	empaiDuration->setPosition(GUI_PADDING_X, empaiDurationLabel->getPosition().y + empaiDurationLabel->getSize().y + GUI_PADDING_Y);
	empaiAngleOffsetLabel->setPosition(tgui::bindLeft(empaiDuration), tgui::bindBottom(empaiDuration) + GUI_PADDING_Y);
	empaiAngleOffset->setPosition(0, tgui::bindBottom(empaiAngleOffsetLabel));
	empaiPolarDistanceLabel->setPosition(tgui::bindLeft(empaiAngleOffsetLabel), tgui::bindBottom(empaiAngleOffset) + GUI_PADDING_Y);
	empaiPolarDistance->setPosition(0, tgui::bindBottom(empaiPolarDistanceLabel));
	empaiPolarAngleLabel->setPosition(tgui::bindLeft(empaiPolarDistanceLabel), tgui::bindBottom(empaiPolarDistance) + GUI_PADDING_Y);
	empaiPolarAngle->setPosition(0, tgui::bindBottom(empaiPolarAngleLabel));
	empaiBezierControlPointsLabel->setPosition(tgui::bindLeft(empaiAngleOffsetLabel), tgui::bindBottom(empaiAngleOffset) + GUI_PADDING_Y);
	empaiBezierControlPoints->setPosition(0, tgui::bindBottom(empaiBezierControlPointsLabel));
	empaiHomingStrengthLabel->setPosition(tgui::bindLeft(empaiDurationLabel), tgui::bindBottom(empaiDuration) + GUI_PADDING_Y);
	empaiHomingStrength->setPosition(0, tgui::bindBottom(empaiHomingStrengthLabel));
	empaiHomingSpeedLabel->setPosition(tgui::bindLeft(empaiHomingStrengthLabel), tgui::bindBottom(empaiDuration) + GUI_PADDING_Y);
	empaiHomingSpeed->setPosition(0, tgui::bindBottom(empaiHomingSpeedLabel));
}

void AttackEditor::selectAttack(int id) {
	deselectEMP();
	if (unsavedAttacks.count(id) > 0) {
		selectedAttack = unsavedAttacks[id];
	} else {
		// Make a copy of the attack in the LevelPack so that changes can be applied/discarded
		// whenever the user wants instead of modifying the LevelPack directly.
		selectedAttack = std::make_shared<EditorAttack>(levelPack.getAttack(id));
	}

	aiName->setReadOnly(false);
	alList->setSelectedItemById(std::to_string(id));
	buildEMPTree();
	alDeleteAttack->setEnabled(true);

	setAttackWidgetValues(selectedAttack, true, false);
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
	emplTree->selectItem(selectedEMP->generatePathToThisEmp(getEMPTreeNodeText));

	bool isBullet = selectedEMP->getIsBullet();
	bool usesModel = selectedEMP->usesBulletModel();
	empiID->setVisible(true);
	empiIsBulletLabel->setVisible(true);
	empiIsBullet->setVisible(true);
	empiHitboxRadiusLabel->setVisible(true);
	empiHitboxRadius->setVisible(true);
	empiDespawnTimeLabel->setVisible(true);
	empiDespawnTime->setVisible(true);
	empiActionsLabel->setVisible(true);
	empiActions->setVisible(true);
	empiActionsAddAbove->setVisible(true);
	empiActionsAddBelow->setVisible(true);
	empiActionsDelete->setVisible(true);
	empiSpawnTypeLabel->setVisible(true);
	empiSpawnType->setVisible(true);
	empiSpawnTypeTimeLabel->setVisible(true);
	empiSpawnTypeTime->setVisible(true);
	empiSpawnTypeXLabel->setVisible(true);
	empiSpawnTypeX->setVisible(true);
	empiSpawnTypeYLabel->setVisible(true);
	empiSpawnTypeY->setVisible(true);
	empiSpawnLocationManualSet->setVisible(true);
	empiShadowTrailLifespanLabel->setVisible(true);
	empiShadowTrailLifespan->setVisible(true);
	empiShadowTrailIntervalLabel->setVisible(true);
	empiShadowTrailInterval->setVisible(true);
	empiAnimatableLabel->setVisible(true);
	empiAnimatable->setVisible(true);
	empiLoopAnimationLabel->setVisible(true);
	empiLoopAnimation->setVisible(true);
	empiBaseSpriteLabel->setVisible(true);
	empiBaseSprite->setVisible(true);
	empiDamageLabel->setVisible(true);
	empiDamage->setVisible(true);
	empiOnCollisionActionLabel->setVisible(true);
	empiOnCollisionAction->setVisible(true);
	empiPierceResetTimeLabel->setVisible(true);
	empiPierceResetTime->setVisible(true);
	empiSoundSettings->setVisible(true);
	empiBulletModelLabel->setVisible(true);
	empiBulletModel->setVisible(true);
	empiInheritRadiusLabel->setVisible(true);
	empiInheritRadius->setVisible(true);
	empiInheritDespawnTimeLabel->setVisible(true);
	empiInheritDespawnTime->setVisible(true);
	empiInheritShadowTrailIntervalLabel->setVisible(true);
	empiInheritShadowTrailInterval->setVisible(true);
	empiInheritShadowTrailLifespanLabel->setVisible(true);
	empiInheritShadowTrailLifespan->setVisible(true);
	empiInheritAnimatablesLabel->setVisible(true);
	empiInheritAnimatables->setVisible(true);
	empiInheritDamageLabel->setVisible(true);
	empiInheritDamage->setVisible(true);
	empiInheritSoundSettingsLabel->setVisible(true);
	empiInheritSoundSettings->setVisible(true);

	empiShadowTrailLifespanLabel->setEnabled(isBullet);
	empiShadowTrailLifespan->setEnabled(isBullet);
	empiShadowTrailIntervalLabel->setEnabled(isBullet);
	empiShadowTrailInterval->setEnabled(isBullet);
	empiAnimatableLabel->setEnabled(isBullet);
	empiAnimatable->setEnabled(isBullet);
	empiLoopAnimationLabel->setEnabled(isBullet);
	empiLoopAnimation->setEnabled(isBullet);
	empiBaseSpriteLabel->setEnabled(isBullet);
	empiBaseSprite->setEnabled(isBullet);
	empiDamageLabel->setEnabled(isBullet);
	empiDamage->setEnabled(isBullet);
	empiOnCollisionActionLabel->setEnabled(isBullet);
	empiOnCollisionAction->setEnabled(isBullet);
	empiBulletModelLabel->setEnabled(isBullet);
	empiBulletModel->setEnabled(isBullet);
	empiInheritRadiusLabel->setEnabled(isBullet && usesModel);
	empiInheritRadius->setEnabled(isBullet && usesModel);
	empiInheritDespawnTimeLabel->setEnabled(isBullet && usesModel);
	empiInheritDespawnTime->setEnabled(isBullet && usesModel);
	empiInheritShadowTrailIntervalLabel->setEnabled(isBullet && usesModel);
	empiInheritShadowTrailInterval->setEnabled(isBullet && usesModel);
	empiInheritShadowTrailLifespanLabel->setEnabled(isBullet && usesModel);
	empiInheritShadowTrailLifespan->setEnabled(isBullet && usesModel);
	empiInheritAnimatablesLabel->setEnabled(isBullet && usesModel);
	empiInheritAnimatables->setEnabled(isBullet && usesModel);
	empiInheritDamageLabel->setEnabled(isBullet && usesModel);
	empiInheritDamage->setEnabled(isBullet && usesModel);
	empiInheritSoundSettingsLabel->setEnabled(isBullet && usesModel);
	empiInheritSoundSettings->setEnabled(isBullet && usesModel);
	empiAnimatable->setEnabled(!usesModel || !selectedEMP->getInheritAnimatables());
	empiLoopAnimation->setEnabled(!usesModel || !selectedEMP->getInheritAnimatables());
	empiBaseSprite->setEnabled(!usesModel || !selectedEMP->getInheritAnimatables());
	empiDamage->setEnabled(!usesModel || !selectedEMP->getInheritDamage());
	empiDespawnTime->setEnabled(!usesModel || !selectedEMP->getInheritDespawnTime());
	empiHitboxRadius->setEnabled(!usesModel || !selectedEMP->getInheritRadius());
	empiShadowTrailInterval->setEnabled(!usesModel || !selectedEMP->getInheritShadowTrailInterval());
	empiShadowTrailLifespan->setEnabled(!usesModel || !selectedEMP->getInheritShadowTrailLifespan());
	empiSoundSettings->setEnabled(!usesModel || !selectedEMP->getInheritSoundSettings());
	empiHitboxRadius->setEnabled(isBullet);

	setEMPWidgetValues(selectedEMP, selectedAttack, true, true);
}

void AttackEditor::deselectEMP() {
	deselectEMPA();
	selectedEMP = nullptr;

	// This is allowed because play area window can only edit EMPA-related stuff
	playAreaWindowUndoStack.clear();

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
	empiActionsAddAbove->setVisible(false);
	empiActionsAddBelow->setVisible(false);
	empiActionsDelete->setVisible(false);
	empiSpawnTypeLabel->setVisible(false);
	empiSpawnType->setVisible(false);
	empiSpawnTypeTimeLabel->setVisible(false);
	empiSpawnTypeTime->setVisible(false);
	empiSpawnTypeXLabel->setVisible(false);
	empiSpawnTypeX->setVisible(false);
	empiSpawnTypeYLabel->setVisible(false);
	empiSpawnTypeY->setVisible(false);
	empiSpawnLocationManualSet->setVisible(false);
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

void AttackEditor::selectEMPA(int index) {
	selectedEMPAIndex = index;
	selectedEMPA = selectedEMP->getActions()[index];
	empiActionsAddAbove->setText("Add above");
	empiActionsAddBelow->setText("Add below");
	empiActionsDelete->setEnabled(true);

	// This is allowed because play area window can only edit EMPA-related stuff
	playAreaWindowUndoStack.clear();

	empaiPanel->setVisible(true);
	if (dynamic_cast<MoveCustomPolarEMPA*>(selectedEMPA.get())) {
		empaiPolarDistance->setVisible(true);
		empaiPolarAngle->setVisible(true);
		empaiBezierControlPoints->setVisible(false);
		empaiAngleOffset->setVisible(true);
		empaiHomingStrength->setVisible(false);
		empaiHomingSpeed->setVisible(false);
		empaiInfo->setText("Attack ID: " + tos(selectedAttack->getID()) + "\nMovable point ID: " + tos(selectedEMP->getID())
			+ "\nPolar coordinates movement action");
	} else if (dynamic_cast<MoveCustomBezierEMPA*>(selectedEMPA.get())) {
		empaiPolarDistance->setVisible(false);
		empaiPolarAngle->setVisible(false);
		empaiBezierControlPoints->setVisible(true);
		empaiAngleOffset->setVisible(true);
		empaiHomingStrength->setVisible(false);
		empaiHomingSpeed->setVisible(false);
		empaiInfo->setText("Attack ID: " + tos(selectedAttack->getID()) + "\nMovable point ID: " + tos(selectedEMP->getID())
			+ "\nBezier movement action");
	} else if (dynamic_cast<MovePlayerHomingEMPA*>(selectedEMPA.get())) {
		empaiPolarDistance->setVisible(false);
		empaiPolarAngle->setVisible(false);
		empaiBezierControlPoints->setVisible(false);
		empaiAngleOffset->setVisible(false);
		empaiHomingStrength->setVisible(true);
		empaiHomingSpeed->setVisible(true);
		empaiInfo->setText("Attack ID: " + tos(selectedAttack->getID()) + "\nMovable point ID: " + tos(selectedEMP->getID())
			+ "\Homing movement action");
	} else if (dynamic_cast<StayStillAtLastPositionEMPA*>(selectedEMPA.get())) {
		empaiPolarDistance->setVisible(false);
		empaiPolarAngle->setVisible(false);
		empaiBezierControlPoints->setVisible(false);
		empaiAngleOffset->setVisible(false);
		empaiHomingStrength->setVisible(false);
		empaiHomingSpeed->setVisible(false);
		empaiInfo->setText("Attack ID: " + tos(selectedAttack->getID()) + "\nMovable point ID: " + tos(selectedEMP->getID())
			+ "\Stay still action");
	} else if (dynamic_cast<DetachFromParentEMPA*>(selectedEMPA.get())) {
		empaiPolarDistance->setVisible(false);
		empaiPolarAngle->setVisible(false);
		empaiBezierControlPoints->setVisible(false);
		empaiAngleOffset->setVisible(false);
		empaiHomingStrength->setVisible(false);
		empaiHomingSpeed->setVisible(false);
		empaiInfo->setText("Attack ID: " + tos(selectedAttack->getID()) + "\nMovable point ID: " + tos(selectedEMP->getID())
			+ "\Detach action");
	} else {
		// This means you forgot to add a case
		assert(false);
	}
	empaiDurationLabel->setVisible(empaiDuration->isVisible());
	empaiPolarDistanceLabel->setVisible(empaiPolarDistance->isVisible());
	empaiPolarAngleLabel->setVisible(empaiPolarAngle->isVisible());
	empaiBezierControlPointsLabel->setVisible(empaiBezierControlPoints->isVisible());
	empaiAngleOffsetLabel->setVisible(empaiAngleOffset->isVisible());
	empaiHomingStrengthLabel->setVisible(empaiHomingStrength->isVisible());
	empaiHomingSpeedLabel->setVisible(empaiHomingSpeed->isVisible());

	empaiDuration->setValue(selectedEMPA->getTime());
}

void AttackEditor::deselectEMPA() {
	selectedEMPAIndex = -1;
	selectedEMPA = nullptr;
	empiActionsAddAbove->setText("Add as first");
	empiActionsAddBelow->setText("Add as last");
	empiActionsDelete->setEnabled(false);

	empaiPanel->setVisible(false);
	empaiInfo->setText("No Movable point action selected");
}

void AttackEditor::setEMPWidgetValues(std::shared_ptr<EditorMovablePoint> emp, std::shared_ptr<EditorAttack> parentAttack, bool skipUndoCommandCreation, bool fromInit) {
	if (!fromInit) {
		setAttackWidgetValues(parentAttack, true, true);
	}
	if (emp != selectedEMP) {
		if (parentAttack != selectedAttack) {
			selectAttack(parentAttack->getID());
		}
		selectEMP(emp->getID());
	}
	this->skipUndoCommandCreation = skipUndoCommandCreation;

	if (!fromInit) {
		buildEMPTree();
		selectEMP(emp->getID());
	}

	empiID->setText("ID: " + std::to_string(emp->getID()));
	empiIsBullet->setChecked(emp->getIsBullet());
	empiSpawnType->setSelectedItemById(getID(emp->getSpawnType()));
	empiSpawnTypeTime->setValue(emp->getSpawnType()->getTime());
	empiSpawnTypeX->setValue(emp->getSpawnType()->getX());
	empiSpawnTypeY->setValue(emp->getSpawnType()->getY());
	empiShadowTrailLifespan->setValue(emp->getShadowTrailLifespan());
	empiOnCollisionAction->setSelectedItemById(getID(emp->getOnCollisionAction()));
	empiPierceResetTime->setValue(emp->getPierceResetTime());
	buildEMPIActions();

	if (emp->usesBulletModel()) {
		empiBulletModel->setSelectedItemById(std::to_string(emp->getBulletModelID()));

		std::shared_ptr<BulletModel> model = levelPack.getBulletModel(emp->getBulletModelID());
		if (emp->getInheritAnimatables()) {
			empiAnimatable->setSelectedItem(model->getAnimatable());
			empiLoopAnimation->setChecked(model->getLoopAnimation());
			empiBaseSprite->setSelectedItem(model->getBaseSprite());
		} else {
			empiAnimatable->setSelectedItem(emp->getAnimatable());
			empiLoopAnimation->setChecked(emp->getLoopAnimation());
			empiBaseSprite->setSelectedItem(emp->getBaseSprite());
		}

		if (emp->getInheritDamage()) {
			empiDamage->setValue(model->getDamage());
		} else {
			empiDamage->setValue(emp->getDamage());
		}

		if (emp->getInheritDespawnTime()) {
			empiDespawnTime->setValue(model->getDespawnTime());
		} else {
			empiDespawnTime->setValue(emp->getDespawnTime());
		}

		if (emp->getInheritRadius()) {
			empiHitboxRadius->setValue(model->getHitboxRadius());
		} else {
			empiHitboxRadius->setValue(emp->getHitboxRadius());
		}

		if (emp->getInheritShadowTrailInterval()) {
			empiShadowTrailInterval->setValue(model->getShadowTrailInterval());
		} else {
			empiShadowTrailInterval->setValue(emp->getShadowTrailInterval());
		}

		if (emp->getInheritShadowTrailLifespan()) {
			empiShadowTrailLifespan->setValue(model->getShadowTrailLifespan());
		} else {
			empiShadowTrailInterval->setValue(emp->getShadowTrailLifespan());
		}

		if (emp->getInheritSoundSettings()) {
			empiSoundSettings->initSettings(model->getSoundSettings());
		} else {
			empiSoundSettings->initSettings(emp->getSoundSettings());
		}
	} else {
		empiAnimatable->setSelectedItem(emp->getAnimatable());
		empiLoopAnimation->setChecked(emp->getLoopAnimation());
		empiBaseSprite->setSelectedItem(emp->getBaseSprite());
		empiDamage->setValue(emp->getDamage());
		empiDespawnTime->setValue(emp->getDespawnTime());
		empiHitboxRadius->setValue(emp->getHitboxRadius());
		empiShadowTrailInterval->setValue(emp->getShadowTrailInterval());
		empiShadowTrailInterval->setValue(emp->getShadowTrailLifespan());
		empiSoundSettings->initSettings(emp->getSoundSettings());
	}

	this->skipUndoCommandCreation = false;
}

void AttackEditor::createAttack() {
	std::shared_ptr<EditorAttack> attack = levelPack.createAttack();
	buildAttackList(true);
}

void AttackEditor::deleteAttack(std::shared_ptr<EditorAttack> attack, bool autoScrollAttackListToBottom) {
	//TODO: prompt: are you sure? this attack may be in use by some attack patterns.

	if (selectedAttack && selectedAttack->getID() == attack->getID()) {
		deselectAttack();
	}
	alList->removeItemById(std::to_string(attack->getID()));
	
	if (unsavedAttacks.count(attack->getID()) > 0) {
		unsavedAttacks.erase(attack->getID());
	}

	levelPack.deleteAttack(attack->getID());
	buildAttackList(autoScrollAttackListToBottom);
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
	if (selectedAttack && selectedAttack->getID() == id) {
		deselectAttack();
	}
}

std::shared_ptr<EditorMovablePoint> AttackEditor::createEMP(std::shared_ptr<EditorAttack> empOwner, std::shared_ptr<EditorMovablePoint> parent) {
	std::shared_ptr<EditorMovablePoint> emp = parent->createChild();
	
	emplTree->addItem(emp->generatePathToThisEmp(&AttackEditor::getEMPTreeNodeText));

	setAttackWidgetValues(empOwner, false, true);

	return emp;
}

void AttackEditor::deleteEMP(std::shared_ptr<EditorAttack> empOwner, std::shared_ptr<EditorMovablePoint> emp) {
	assert(emp->getID() != empOwner->getMainEMP()->getID());

	emp->detachFromParent();
	buildEMPTree();

	setAttackWidgetValues(empOwner, false, true);
}

void AttackEditor::setAttackWidgetValues(std::shared_ptr<EditorAttack> attackWithUnsavedChanges, bool skipUndoCommandCreation, bool attackWasModified) {
	if (attackWithUnsavedChanges != selectedAttack) {
		selectAttack(attackWithUnsavedChanges->getID());
	}

	int id = attackWithUnsavedChanges->getID();
	if (attackWasModified) {
		unsavedAttacks[id] = attackWithUnsavedChanges;
	}
	
	// Add asterisk to the entry in attack list
	if (attackWasModified && unsavedAttacks.count(id) > 0) {
		alList->changeItemById(std::to_string(id), "*" + attackWithUnsavedChanges->getName() + " [id=" + std::to_string(id) + "]");
	}

	this->skipUndoCommandCreation = skipUndoCommandCreation;
	aiID->setText("Attack ID: " + std::to_string(id));
	aiName->setText(selectedAttack->getName());
	aiPlayAttackAnimation->setChecked(selectedAttack->getPlayAttackAnimation());
	skipUndoCommandCreation = false;
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

void AttackEditor::buildAttackList(bool autoscrollToBottom) {
	alList->removeAllItems();
	if (!autoscrollToBottom) {
		alList->setAutoScroll(false);
	}
	for (auto it = levelPack.getAttackIteratorBegin(); it != levelPack.getAttackIteratorEnd(); it++) {
		if (unsavedAttacks.count(it->second->getID()) > 0) {
			alList->addItem("*" + it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
		} else {
			alList->addItem(it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
		}
	}
	alList->setAutoScroll(true);
}

void AttackEditor::onMainWindowRender(float deltaTime) {
	std::lock_guard<std::mutex> lock(*tguiMutex);

	empiAnimatable->getAnimatablePicture()->update(deltaTime);
	empiBaseSprite->getAnimatablePicture()->update(deltaTime);
}

void AttackEditor::onEmpiHitboxRadiusChange(float value) {
	selectedEMP->setHitboxRadius(value);
	setAttackWidgetValues(selectedAttack, false, true);
}

void AttackEditor::onEmpiDespawnTimeChange(float value) {
	selectedEMP->setDespawnTime(value);
	setAttackWidgetValues(selectedAttack, false, true);
}
