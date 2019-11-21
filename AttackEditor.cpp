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

AttackEditor::AttackEditor(std::shared_ptr<LevelPack> levelPack, std::shared_ptr<SpriteLoader> spriteLoader) : levelPack(levelPack), spriteLoader(spriteLoader), mainWindowUndoStack(UndoStack(UNDO_STACK_MAX)) {
	tguiMutex = std::make_shared<std::recursive_mutex>();

	mainWindow = std::make_shared<AttackEditorMainWindow>(tguiMutex, "Attack Editor", MAIN_WINDOW_WIDTH, 400, mainWindowUndoStack);
	gameplayTestWindow = std::make_shared<GameplayTestWindow>(levelPack, spriteLoader, tguiMutex, "Attack Editor - Gameplay Test", MAP_WIDTH, MAP_HEIGHT);

	std::lock_guard<std::recursive_mutex> lock(*tguiMutex);

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
				setAttackWidgetValues(selectedAttack, true);
			},
				[this, selectedAttack, oldName]() {
				selectedAttack->setName(oldName);
				setAttackWidgetValues(selectedAttack, true);
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
				setAttackWidgetValues(selectedAttack, true);
			},
				[this, selectedAttack, checked]() {
				selectedAttack->setPlayAttackAnimation(!checked);
				setAttackWidgetValues(selectedAttack, true);
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
	alList = std::make_shared<ScrollableListBox>();
	alSaveAll = tgui::Button::create();
	alDiscardAll = tgui::Button::create();
	alCreateAttack = tgui::Button::create();
	alDeleteAttack = tgui::Button::create();

	alList->setTextSize(TEXT_SIZE);
	alList->getListBox()->setItemHeight(TEXT_BOX_HEIGHT);
	alList->getListBox()->setMaximumItems(0);

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

	alList->getListBox()->connect("ItemSelected", [&](std::string itemName, std::string itemID) {
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
		this->levelPack->save();
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
		std::shared_ptr<EditorAttack> newAttack = levelPack->createAttack(false);
		mainWindowUndoStack.execute(UndoableCommand(
			[this, &levelPack, newAttack]() {
			levelPack->updateAttack(newAttack);
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
			levelPack->updateAttack(selectedAttack);
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
	emplTestEMP = tgui::Button::create();

	emplTree->getRenderer()->setBackgroundColor(sf::Color(190, 190, 190, 255));
	emplTree->setItemHeight(TEXT_BOX_HEIGHT);

	emplCreateEMP->setTextSize(TEXT_SIZE);
	emplDeleteEMP->setTextSize(TEXT_SIZE);
	emplTestEMP->setTextSize(TEXT_SIZE);
	emplLabel->setMaximumTextWidth(0);
	emplLabel->setTextSize(TEXT_SIZE);
	emplLabel->setText("Movable points");

	emplCreateEMP->setText("New");
	emplDeleteEMP->setText("Delete");
	emplTestEMP->setText("Test EMP");

	emplCreateEMP->setToolTip(createToolTip("Create a new movable point"));
	emplDeleteEMP->setToolTip(createToolTip("Delete the selected movable point"));
	emplTestEMP->setToolTip(createToolTip("Tests the EMP's movement. The EMP must be legal to be tested. Note that only the movement actions are tested, \
		not anything else. In the test, the EMP will not be attached to any parent."));
	
	emplTree->connect("ItemSelected", [&](sf::String nodeText) {
		if (nodeText != "") {
			std::string str = nodeText;
			// Extract the "[id=___]" portion out of the node text
			int idBeginPos = str.find_last_of('=') + 1;
			selectEMP(selectedAttack, std::stoi(str.substr(idBeginPos, str.length() - idBeginPos - 1)));
		}
	});
	emplCreateEMP->connect("Pressed", [this, &selectedEMP = this->selectedEMP]() {
		std::shared_ptr<EditorMovablePoint> emp = createEMP(selectedAttack, selectedEMP, false);
		mainWindowUndoStack.execute(UndoableCommand(
			[this, emp, selectedEMP]() {
			selectedEMP->addChild(emp);
			buildEMPTree();
		},
			[this, emp, selectedEMP]() {
			if (this->selectedEMP == emp) {
				deselectEMP();
			}
			emp->detachFromParent();
			buildEMPTree();
		}));
	});
	emplDeleteEMP->connect("Pressed", [this, &selectedAttack = this->selectedAttack, &selectedEMP = this->selectedEMP]() {
		// Can't delete main EMP of an attack
		if (selectedEMP && selectedEMP->getID() != selectedAttack->getMainEMP()->getID()) {
			mainWindowUndoStack.execute(UndoableCommand(
				[this, selectedAttack, selectedEMP]() {
				deleteEMP(selectedAttack, selectedEMP);
			},
				[this, selectedEMP]() {
				selectedEMP->getParent()->addChild(selectedEMP);
				buildEMPTree();
			}));
		}
	});
	emplTestEMP->connect("Pressed", [this, &selectedAttack = this->selectedAttack, &selectedEMP = this->selectedEMP]() {
		sendToForeground(*gameplayTestWindow->getWindow());
		gameplayTestWindow->addEMPTestPlaceholder(selectedEMP, true, selectedAttack->getID());
	});

	emplPanel->add(emplLabel);
	emplPanel->add(emplTree);
	emplPanel->add(emplCreateEMP);
	emplPanel->add(emplDeleteEMP);
	emplPanel->add(emplTestEMP);
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
	empiActions = std::make_shared<ScrollableListBox>();
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
	empiSoundSettings = std::make_shared<SoundSettingsGroup>("Level Packs\\" + levelPack->getName() + "\\Sounds", GUI_PADDING_X, GUI_PADDING_Y);
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

	empiSpawnType->setChangeItemOnScroll(false);
	empiOnCollisionAction->setChangeItemOnScroll(false);
	empiBulletModel->setChangeItemOnScroll(false);
	empiActions->getListBox()->setItemHeight(20);

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
		if (ignoreSignal) return;
		bool checked = empiIsBullet->isChecked();
		if (checked != selectedEMP->getIsBullet()) {
			mainWindowUndoStack.execute(UndoableCommand(
				[this, selectedEMP, selectedAttack, checked]() {
				selectedEMP->setIsBullet(checked);
				setEMPWidgetValues(selectedEMP, selectedAttack, false);
			},
				[this, selectedEMP, selectedAttack, checked]() {
				selectedEMP->setIsBullet(!checked);
				setEMPWidgetValues(selectedEMP, selectedAttack, false);
			}));
		}
	});
	empiHitboxRadius->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onEmpiHitboxRadiusChange>(this);
	empiDespawnTime->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onEmpiDespawnTimeChange>(this);
	empiActions->getListBox()->connect("MousePressed", [&](std::string item, std::string id) {
		if (ignoreSignal) return;
		if (id == "") {
			deselectEMPA();
		} else {
			selectEMPA(std::stoi(id));
		}
	});
	empiActionsAddAbove->connect("Pressed", [this, &selectedEMP = this->selectedEMP, &selectedEMPAIndex = this->selectedEMPAIndex, &selectedAttack = this->selectedAttack]() {
		if (ignoreSignal) return;
		auto popup = tgui::ListBox::create();
		popup->addItem("Bezier movement", "1");
		popup->addItem("Polar movement", "2");
		popup->addItem("Homing movement", "3");
		popup->addItem("Stay still", "4");
		popup->addItem("Detach", "5");
		popup->setItemHeight(20);
		popup->setPosition(empiActionsAddAbove->getPosition());
		popup->setSize(150, popup->getItemHeight() * popup->getItemCount());
		popup->connect("MousePressed", [this, selectedEMP, selectedAttack, selectedEMPAIndex](std::string item, std::string id) {
			if (ignoreSignal) return;
			int index;
			if (selectedEMPAIndex == -1) {
				index = 0;
			} else {
				index = selectedEMPAIndex;
			}
			if (id == "1") {
				mainWindowUndoStack.execute(UndoableCommand(
					[this, index, selectedEMP, selectedAttack]() {
					auto empa = std::make_shared<MoveCustomBezierEMPA>(std::vector<sf::Vector2f>{sf::Vector2f(0, 0), sf::Vector2f(100, 0)}, 3);
					selectedEMP->insertAction(index, empa);
					selectEMPA(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
					setEMPAWidgetValues(empa, selectedEMP, selectedAttack);
				},
					[this, index, selectedEMP, selectedAttack]() {
					selectedEMP->removeAction(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
				}));
			} else if (id == "2") {
				mainWindowUndoStack.execute(UndoableCommand(
					[this, index, selectedEMP, selectedAttack]() {
					auto empa = std::make_shared<MoveCustomPolarEMPA>(std::make_shared<ConstantTFV>(0), std::make_shared<ConstantTFV>(0), 3);
					selectedEMP->insertAction(index, empa);
					selectEMPA(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
					setEMPAWidgetValues(empa, selectedEMP, selectedAttack);
				},
					[this, index, selectedEMP, selectedAttack]() {
					selectedEMP->removeAction(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false); buildEMPIActions();
				}));
			} else if (id == "3") {
				mainWindowUndoStack.execute(UndoableCommand(
					[this, index, selectedEMP, selectedAttack]() {
					auto empa = std::make_shared<MovePlayerHomingEMPA>(std::make_shared<ConstantTFV>(0), std::make_shared<ConstantTFV>(0), 3);
					selectedEMP->insertAction(index, empa);
					selectEMPA(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
					setEMPAWidgetValues(empa, selectedEMP, selectedAttack);
				},
					[this, index, selectedEMP, selectedAttack]() {
					selectedEMP->removeAction(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
				}));
			} else if (id == "4") {
				mainWindowUndoStack.execute(UndoableCommand(
					[this, index, selectedEMP, selectedAttack]() {
					auto empa = std::make_shared<StayStillAtLastPositionEMPA>(3);
					selectedEMP->insertAction(index, empa);
					selectEMPA(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
					setEMPAWidgetValues(empa, selectedEMP, selectedAttack);
				},
					[this, index, selectedEMP, selectedAttack]() {
					selectedEMP->removeAction(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
				}));
			} else if (id == "5") {
				mainWindowUndoStack.execute(UndoableCommand(
					[this, index, selectedEMP, selectedAttack]() {
					auto empa = std::make_shared<DetachFromParentEMPA>();
					selectedEMP->insertAction(index, empa);
					selectEMPA(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
					setEMPAWidgetValues(empa, selectedEMP, selectedAttack);
				},
					[this, index, selectedEMP, selectedAttack]() {
					selectedEMP->removeAction(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
				}));
			}
		});
		mainWindow->addPopupWidget(empiPanel, popup);
	});
	empiActionsAddBelow->connect("Pressed", [this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, &selectedEMPAIndex = this->selectedEMPAIndex]() {
		if (ignoreSignal) return;
		auto popup = tgui::ListBox::create();
		popup->addItem("Bezier movement", "1");
		popup->addItem("Polar movement", "2");
		popup->addItem("Homing movement", "3");
		popup->addItem("Stay still", "4");
		popup->addItem("Detach", "5");
		popup->setItemHeight(20);
		popup->setPosition(empiActionsAddBelow->getPosition());
		popup->setSize(150, popup->getItemHeight() * popup->getItemCount());
		popup->connect("MousePressed", [this, selectedEMP, selectedAttack, selectedEMPAIndex](std::string item, std::string id) {
			if (ignoreSignal) return;
			int index;
			if (selectedEMPAIndex == -1) {
				index = selectedEMP->getActions().size();
			} else {
				index = selectedEMPAIndex + 1;
			}
			if (id == "1") {
				mainWindowUndoStack.execute(UndoableCommand(
					[this, index, selectedEMP, selectedAttack]() {
					auto empa = std::make_shared<MoveCustomBezierEMPA>(std::vector<sf::Vector2f>{sf::Vector2f(0, 0), sf::Vector2f(100, 0)}, 3);
					selectedEMP->insertAction(index, empa);
					selectEMPA(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
					setEMPAWidgetValues(empa, selectedEMP, selectedAttack);
				},
					[this, index, selectedEMP, selectedAttack]() {
					selectedEMP->removeAction(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
				}));
			} else if (id == "2") {
				mainWindowUndoStack.execute(UndoableCommand(
					[this, index, selectedEMP, selectedAttack]() {
					auto empa = std::make_shared<MoveCustomPolarEMPA>(std::make_shared<ConstantTFV>(0), std::make_shared<ConstantTFV>(0), 3);
					selectedEMP->insertAction(index, empa);
					selectEMPA(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
					setEMPAWidgetValues(empa, selectedEMP, selectedAttack);
				},
					[this, index, selectedEMP, selectedAttack]() {
					selectedEMP->removeAction(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false); buildEMPIActions();
				}));
			} else if (id == "3") {
				mainWindowUndoStack.execute(UndoableCommand(
					[this, index, selectedEMP, selectedAttack]() {
					auto empa = std::make_shared<MovePlayerHomingEMPA>(std::make_shared<ConstantTFV>(0), std::make_shared<ConstantTFV>(0), 3);
					selectedEMP->insertAction(index, empa);
					selectEMPA(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
					setEMPAWidgetValues(empa, selectedEMP, selectedAttack);
				},
					[this, index, selectedEMP, selectedAttack]() {
					selectedEMP->removeAction(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
				}));
			} else if (id == "4") {
				mainWindowUndoStack.execute(UndoableCommand(
					[this, index, selectedEMP, selectedAttack]() {
					auto empa = std::make_shared<StayStillAtLastPositionEMPA>(3);
					selectedEMP->insertAction(index, empa);
					selectEMPA(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
					setEMPAWidgetValues(empa, selectedEMP, selectedAttack);
				},
					[this, index, selectedEMP, selectedAttack]() {
					selectedEMP->removeAction(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
				}));
			} else if (id == "5") {
				mainWindowUndoStack.execute(UndoableCommand(
					[this, index, selectedEMP, selectedAttack]() {
					auto empa = std::make_shared<DetachFromParentEMPA>();
					selectedEMP->insertAction(index, empa);
					selectEMPA(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
					setEMPAWidgetValues(empa, selectedEMP, selectedAttack);
				},
					[this, index, selectedEMP, selectedAttack]() {
					selectedEMP->removeAction(index);
					setEMPWidgetValues(selectedEMP, selectedAttack, false);
				}));
			}
		});
		mainWindow->addPopupWidget(empiPanel, popup);
	});
	empiActionsDelete->connect("Pressed", [this, &selectedEMP = this->selectedEMP, &selectedEMPA = this->selectedEMPA, &selectedEMPAIndex = this->selectedEMPAIndex]() {
		if (ignoreSignal) return;
		mainWindowUndoStack.execute(UndoableCommand(
			[this, selectedEMP, selectedEMPAIndex]() {
			deleteEMPA(selectedEMP, selectedEMPAIndex);
		},
			[this, selectedEMP, selectedEMPAIndex, selectedEMPA]() {
			selectedEMP->insertAction(selectedEMPAIndex, selectedEMPA);
			buildEMPIActions();
		}));
	});

	// See getID(EMPSpawnType*) for which ID to use
	empiSpawnType->addItem("Relative to map", "0");
	empiSpawnType->addItem("Relative to parent, detached", "1");
	empiSpawnType->addItem("Relative to parent, attached", "2");
	empiSpawnType->connect("ItemSelected", [this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack](std::string item, std::string id) {
		if (ignoreSignal) return;
		std::shared_ptr<EMPSpawnType> oldSpawnType = selectedEMP->getSpawnType();
		if (id == "1") {
			mainWindowUndoStack.execute(UndoableCommand(
				[this, selectedEMP, selectedAttack]() {
				selectedEMP->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>(empiSpawnTypeTime->getValue(), empiSpawnTypeX->getValue(), empiSpawnTypeY->getValue()));
				setEMPWidgetValues(selectedEMP, selectedAttack, false);
			},
				[this, selectedEMP, selectedAttack, oldSpawnType]() {
				selectedEMP->setSpawnType(oldSpawnType);
				setEMPWidgetValues(selectedEMP, selectedAttack, false);
			}));
		} else if (id == "2") {
			mainWindowUndoStack.execute(UndoableCommand(
				[this, selectedEMP, selectedAttack]() {
				selectedEMP->setSpawnType(std::make_shared<EntityAttachedEMPSpawn>(empiSpawnTypeTime->getValue(), empiSpawnTypeX->getValue(), empiSpawnTypeY->getValue()));
				setEMPWidgetValues(selectedEMP, selectedAttack, false);
			},
				[this, selectedEMP, selectedAttack, oldSpawnType]() {
				selectedEMP->setSpawnType(oldSpawnType);
				setEMPWidgetValues(selectedEMP, selectedAttack, false);
			}));
		} else {
			mainWindowUndoStack.execute(UndoableCommand(
				[this, selectedEMP, selectedAttack]() {
				selectedEMP->setSpawnType(std::make_shared<SpecificGlobalEMPSpawn>(empiSpawnTypeTime->getValue(), empiSpawnTypeX->getValue(), empiSpawnTypeY->getValue()));
				setEMPWidgetValues(selectedEMP, selectedAttack, false);
			},
				[this, selectedEMP, selectedAttack, oldSpawnType]() {
				selectedEMP->setSpawnType(oldSpawnType);
				setEMPWidgetValues(selectedEMP, selectedAttack, false);
			}));
		}
	});
	empiSpawnTypeTime->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onEmpiSpawnTypeTimeChange>(this);
	empiSpawnTypeX->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onEmpiSpawnTypeXChange>(this);
	empiSpawnTypeY->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onEmpiSpawnTypeYChange>(this);
	empiSpawnLocationManualSet->connect("Pressed", [&]() {
		//TODO: next click on play area sets empiSpawnTypeX/Y values
	});
	empiShadowTrailLifespan->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onEmpiShadowTrailLifespanChange>(this);
	empiShadowTrailInterval->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onEmpiShadowTrailIntervalChange>(this);
	empiAnimatable->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onAnimatableChange>(this);
	empiLoopAnimation->connect("Checked", [this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack]() {
		if (ignoreSignal) return;
		bool checked = empiLoopAnimation->isChecked();
		mainWindowUndoStack.execute(UndoableCommand(
			[this, selectedEMP, selectedAttack, checked]() {
			selectedEMP->setLoopAnimation(checked);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		},
			[this, selectedEMP, selectedAttack, checked]() {
			selectedEMP->setLoopAnimation(!checked);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		}));
	});
	empiBaseSprite->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onBaseSpriteChange>(this);
	empiDamage->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onEmpiDamageChange>(this);
	
	empiOnCollisionAction->addItem("Destroy self only", getID(DESTROY_THIS_BULLET_ONLY));
	empiOnCollisionAction->addItem("Destroy self and attached children", getID(DESTROY_THIS_BULLET_AND_ATTACHED_CHILDREN));
	empiOnCollisionAction->addItem("Pierce targets", getID(PIERCE_ENTITY));
	empiOnCollisionAction->connect("ItemSelected", [this, levelPack, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack](std::string item, std::string id) {
		if (ignoreSignal) return;
		BULLET_ON_COLLISION_ACTION action = static_cast<BULLET_ON_COLLISION_ACTION>(std::stoi(std::string(empiOnCollisionAction->getSelectedItemId())));
		BULLET_ON_COLLISION_ACTION oldAction = selectedEMP->getOnCollisionAction();
		mainWindowUndoStack.execute(UndoableCommand(
			[this, selectedEMP, selectedAttack, action]() {
			selectedEMP->setOnCollisionAction(action);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		},
			[this, selectedEMP, selectedAttack, oldAction]() {
			selectedEMP->setOnCollisionAction(oldAction);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		}));
	});

	empiPierceResetTime->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onEmpiPierceResetTimeChange>(this);
	empiSoundSettings->getOnNewSoundSettingsSignal()->sink().connect<AttackEditor, &AttackEditor::onEmpiSoundSettingsChange>(this);
	empiBulletModel->connect("ItemSelected", [this, levelPack, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack](std::string item, std::string id) {
		if (ignoreSignal) return;
		int bulletModelID = std::stoi(id);
		if (item == "") bulletModelID = -1;
		int oldBulletModelID = selectedEMP->getBulletModelID();
		float radius = selectedEMP->getHitboxRadius();
		float despawnTime = selectedEMP->getDespawnTime();
		float interval = selectedEMP->getShadowTrailInterval();
		float lifespan = selectedEMP->getShadowTrailLifespan();
		Animatable animatable = selectedEMP->getAnimatable();
		Animatable baseSprite = selectedEMP->getBaseSprite();
		bool loopAnimation = selectedEMP->getLoopAnimation();
		float damage = selectedEMP->getDamage();
		SoundSettings sound = selectedEMP->getSoundSettings();
		mainWindowUndoStack.execute(UndoableCommand(
			[this, levelPack, selectedEMP, selectedAttack, bulletModelID]() {
			if (bulletModelID == -1) {
				selectedEMP->removeBulletModel();
			} else {
				selectedEMP->setBulletModel(levelPack->getBulletModel(bulletModelID));
			}
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		},
			[this, levelPack, selectedEMP, selectedAttack, oldBulletModelID,
			radius, despawnTime, interval, lifespan, animatable, baseSprite,
			loopAnimation, damage, sound]() {
			selectedEMP->setHitboxRadius(radius);
			selectedEMP->setDespawnTime(despawnTime);
			selectedEMP->setShadowTrailInterval(interval);
			selectedEMP->setShadowTrailLifespan(lifespan);
			selectedEMP->setAnimatable(animatable);
			selectedEMP->setLoopAnimation(loopAnimation);
			selectedEMP->setBaseSprite(baseSprite);
			selectedEMP->setDamage(damage);
			selectedEMP->setSoundSettings(sound);
			if (oldBulletModelID == -1) {
				selectedEMP->removeBulletModel();
			} else {
				selectedEMP->setBulletModel(levelPack->getBulletModel(oldBulletModelID));
			}
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		}));
	});
	empiInheritRadius->connect("Changed", [this, levelPack, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack]() {
		if (ignoreSignal) return;
		bool checked = empiInheritRadius->isChecked();
		float oldValue = selectedEMP->getHitboxRadius();
		mainWindowUndoStack.execute(UndoableCommand(
			[this, levelPack, selectedEMP, selectedAttack, checked, oldValue]() {
			selectedEMP->setHitboxRadius(oldValue);
			selectedEMP->setInheritRadius(checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		},
			[this, levelPack, selectedEMP, selectedAttack, checked, oldValue]() {
			selectedEMP->setHitboxRadius(oldValue);
			selectedEMP->setInheritRadius(!checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		}));
	});
	empiInheritDespawnTime->connect("Changed", [this, levelPack, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack]() {
		if (ignoreSignal) return;
		bool checked = empiInheritDespawnTime->isChecked();
		float oldValue = selectedEMP->getDespawnTime();
		mainWindowUndoStack.execute(UndoableCommand(
			[this, levelPack, selectedEMP, selectedAttack, checked, oldValue]() {
			selectedEMP->setDespawnTime(oldValue);
			selectedEMP->setInheritDespawnTime(checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		},
			[this, levelPack, selectedEMP, selectedAttack, checked, oldValue]() {
			selectedEMP->setDespawnTime(oldValue);
			selectedEMP->setInheritDespawnTime(!checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		}));
	});
	empiInheritShadowTrailInterval->connect("Changed", [this, levelPack, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack]() {
		if (ignoreSignal) return;
		bool checked = empiInheritShadowTrailInterval->isChecked();
		float oldValue = selectedEMP->getShadowTrailInterval();
		mainWindowUndoStack.execute(UndoableCommand(
			[this, levelPack, selectedEMP, selectedAttack, checked, oldValue]() {
			selectedEMP->setShadowTrailInterval(oldValue);
			selectedEMP->setInheritShadowTrailInterval(checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		},
			[this, levelPack, selectedEMP, selectedAttack, checked, oldValue]() {
			selectedEMP->setShadowTrailInterval(oldValue);
			selectedEMP->setInheritShadowTrailInterval(!checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		}));
	});
	empiInheritShadowTrailLifespan->connect("Changed", [this, levelPack, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack]() {
		if (ignoreSignal) return;
		bool checked = empiInheritShadowTrailLifespan->isChecked();
		float oldValue = selectedEMP->getShadowTrailLifespan();
		mainWindowUndoStack.execute(UndoableCommand(
			[this, levelPack, selectedEMP, selectedAttack, checked, oldValue]() {
			selectedEMP->setShadowTrailLifespan(oldValue);
			selectedEMP->setInheritShadowTrailLifespan(checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		},
			[this, levelPack, selectedEMP, selectedAttack, checked, oldValue]() {
			selectedEMP->setShadowTrailLifespan(oldValue);
			selectedEMP->setInheritShadowTrailLifespan(!checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		}));
	});
	empiInheritAnimatables->connect("Changed", [this, levelPack, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack]() {
		if (ignoreSignal) return;
		bool checked = empiInheritAnimatables->isChecked();
		Animatable oldAnimatable = selectedEMP->getAnimatable();
		Animatable oldBaseSprite = selectedEMP->getBaseSprite();
		bool oldLoop = selectedEMP->getLoopAnimation();
		mainWindowUndoStack.execute(UndoableCommand(
			[this, levelPack, selectedEMP, selectedAttack, checked, oldAnimatable, oldBaseSprite, oldLoop]() {
			selectedEMP->setAnimatable(oldAnimatable);
			selectedEMP->setLoopAnimation(oldLoop);
			selectedEMP->setBaseSprite(oldBaseSprite);
			selectedEMP->setInheritAnimatables(checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		},
			[this, levelPack, selectedEMP, selectedAttack, checked, oldAnimatable, oldBaseSprite, oldLoop]() {
			selectedEMP->setAnimatable(oldAnimatable);
			selectedEMP->setLoopAnimation(oldLoop);
			selectedEMP->setBaseSprite(oldBaseSprite);
			selectedEMP->setInheritAnimatables(!checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		}));
	});
	empiInheritDamage->connect("Changed", [this, levelPack, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack]() {
		if (ignoreSignal) return;
		bool checked = empiInheritDamage->isChecked();
		float oldValue = selectedEMP->getDamage();
		mainWindowUndoStack.execute(UndoableCommand(
			[this, levelPack, selectedEMP, selectedAttack, checked, oldValue]() {
			selectedEMP->setDamage(oldValue);
			selectedEMP->setInheritDamage(checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		},
			[this, levelPack, selectedEMP, selectedAttack, checked, oldValue]() {
			selectedEMP->setDamage(oldValue);
			selectedEMP->setInheritDamage(!checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		}));
	});
	empiInheritSoundSettings->connect("Changed", [this, levelPack, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack]() {
		if (ignoreSignal) return;
		bool checked = empiInheritSoundSettings->isChecked();
		SoundSettings oldValue = selectedEMP->getSoundSettings();
		mainWindowUndoStack.execute(UndoableCommand(
			[this, levelPack, selectedEMP, selectedAttack, checked, oldValue]() {
			selectedEMP->setSoundSettings(oldValue);
			selectedEMP->setInheritSoundSettings(checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		},
			[this, levelPack, selectedEMP, selectedAttack, checked, oldValue]() {
			selectedEMP->setSoundSettings(oldValue);
			selectedEMP->setInheritSoundSettings(!checked, *levelPack);
			setEMPWidgetValues(selectedEMP, selectedAttack, false);
		}));
	});

	empiBulletModel->removeAllItems();
	empiBulletModel->addItem("", "-1");
	for (auto it = levelPack->getBulletModelIteratorBegin(); it != levelPack->getBulletModelIteratorEnd(); it++) {
		empiBulletModel->addItem(it->second->getName(), std::to_string(it->second->getID()));
	}

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

	//------------------ EMPA info widgets ---------------------------------------
	empaiPanel = tgui::ScrollablePanel::create();
	empaiInfo = tgui::Label::create();
	empaiDurationLabel = tgui::Label::create();
	empaiDuration = std::make_shared<SliderWithEditBox>();
	empaiPolarDistanceLabel = tgui::Label::create();
	empaiPolarDistance = std::make_shared<TFVGroup>(tguiMutex);
	empaiPolarAngleLabel = tgui::Label::create();
	empaiPolarAngle = std::make_shared<TFVGroup>(tguiMutex);
	empaiBezierControlPointsLabel = tgui::Label::create();
	empaiBezierControlPoints = std::make_shared<ScrollableListBox>();
	empaiAngleOffsetLabel = tgui::Label::create();
	empaiAngleOffset = std::make_shared<EMPAAngleOffsetGroup>();
	empaiHomingStrengthLabel = tgui::Label::create();
	empaiHomingStrength = std::make_shared<TFVGroup>(tguiMutex);
	empaiHomingSpeedLabel = tgui::Label::create();
	empaiHomingSpeed = std::make_shared<TFVGroup>(tguiMutex);
	empaiEditBezierControlPoints = tgui::Button::create();

	empaiInfo->setTextSize(TEXT_SIZE);
	empaiDurationLabel->setTextSize(TEXT_SIZE);
	empaiPolarDistanceLabel->setTextSize(TEXT_SIZE);
	empaiPolarAngleLabel->setTextSize(TEXT_SIZE);
	empaiBezierControlPointsLabel->setTextSize(TEXT_SIZE);
	empaiAngleOffsetLabel->setTextSize(TEXT_SIZE);
	empaiHomingStrengthLabel->setTextSize(TEXT_SIZE);
	empaiHomingSpeedLabel->setTextSize(TEXT_SIZE);
	empaiEditBezierControlPoints->setTextSize(TEXT_SIZE);

	empaiDurationLabel->setText("Duration");
	empaiPolarDistanceLabel->setText("Distance as function of time");
	empaiPolarAngleLabel->setText("Angle as function of time");
	empaiBezierControlPointsLabel->setText("Bezier control points");
	empaiAngleOffsetLabel->setText("Angle offset");
	empaiHomingStrengthLabel->setText("Homing strength as function of time");
	empaiHomingSpeedLabel->setText("Speed as function of time");
	empaiEditBezierControlPoints->setText("Edit control points");

	empaiDuration->getOnValueSet()->sink().connect<AttackEditor, &AttackEditor::onEmpaiDurationChange>(this);
	empaiAngleOffset->getOnAngleOffsetChange()->sink().connect<AttackEditor, &AttackEditor::onEmpaiAngleOffsetChange>(this);
	empaiEditBezierControlPoints->connect("Pressed", [&]() {
		editingEMPABezierControlPoints = true;

		// Block all user input
		ibPanel->setVisible(true);
		ibText->setText("Currently editing bezier control points for Attack ID " + std::to_string(selectedAttack->getID()) + ", EMP ID " + 
			std::to_string(selectedEMP->getID()) + ", EMPA #" + std::to_string(selectedEMPAIndex + 1) + " in the gameplay test window.");
		
		gameplayTestWindow->beginEditingBezierControlPoints(dynamic_cast<MoveCustomBezierEMPA*>(selectedEMPA.get()));
		sendToForeground(*gameplayTestWindow->getWindow());
	});

	empaiPolarDistance->getOnEditingEnd()->sink().connect<AttackEditor, &AttackEditor::onTFVEditingEnd>(this);
	empaiPolarAngle->getOnEditingEnd()->sink().connect<AttackEditor, &AttackEditor::onTFVEditingEnd>(this);
	empaiHomingStrength->getOnEditingEnd()->sink().connect<AttackEditor, &AttackEditor::onTFVEditingEnd>(this);
	empaiHomingSpeed->getOnEditingEnd()->sink().connect<AttackEditor, &AttackEditor::onTFVEditingEnd>(this);
	empaiPolarDistance->getOnEditingStart()->sink().connect<AttackEditor, &AttackEditor::onTFVEditingStart>(this);
	empaiPolarAngle->getOnEditingStart()->sink().connect<AttackEditor, &AttackEditor::onTFVEditingStart>(this);
	empaiHomingStrength->getOnEditingStart()->sink().connect<AttackEditor, &AttackEditor::onTFVEditingStart>(this);
	empaiHomingSpeed->getOnSave()->sink().connect<AttackEditor, &AttackEditor::onTFVEditingSave>(this);
	empaiPolarDistance->getOnSave()->sink().connect<AttackEditor, &AttackEditor::onTFVEditingSave>(this);
	empaiPolarAngle->getOnSave()->sink().connect<AttackEditor, &AttackEditor::onTFVEditingSave>(this);
	empaiHomingStrength->getOnSave()->sink().connect<AttackEditor, &AttackEditor::onTFVEditingSave>(this);
	empaiHomingSpeed->getOnSave()->sink().connect<AttackEditor, &AttackEditor::onTFVEditingSave>(this);

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
	empaiPanel->add(empaiEditBezierControlPoints);
	//------------------ Input blocker widgets ---------------------------------------
	ibPanel = tgui::Panel::create();
	ibText = tgui::Label::create();

	ibPanel->setVisible(false);
	ibText->setTextSize(TEXT_SIZE);

	ibPanel->add(ibText);
	//----------------------------------------------------------------------------

	mainWindow->getResizeSignal()->sink().connect<AttackEditor, &AttackEditor::onMainWindowResize>(this);
	onMainWindowResize(mainWindow->getWindowWidth(), mainWindow->getWindowHeight());
	gameplayTestWindow->getCloseSignal()->sink().connect<AttackEditor, &AttackEditor::onGameplayTestWindowClose>(this);
	gameplayTestWindow->getOnBezierControlPointEditingEndSignal()->sink().connect<AttackEditor, &AttackEditor::onGameplayTestWindowBezierControlPointsEditingEnd>(this);
}

void AttackEditor::start() {
	mainWindowThread = std::thread(&EditorWindow::start, mainWindow);
	playAreaWindowThread = std::thread(&EditorWindow::start, gameplayTestWindow);

	mainWindowThread.detach();
	playAreaWindowThread.detach();

	mainWindow->getRenderSignal()->sink().connect<AttackEditor, &AttackEditor::onMainWindowRender>(this);

	std::shared_ptr<tgui::Gui> mainWindowGui = mainWindow->getGui();
	mainWindowGui->add(aiPanel);
	mainWindowGui->add(alPanel);
	mainWindowGui->add(emplPanel);
	mainWindowGui->add(empiPanel);
	mainWindowGui->add(empaiPanel);
	mainWindowGui->add(ibPanel);

	deselectAttack();
}

void AttackEditor::close() {
	//TODO: if there are unsaved changes, create a "are you sure" prompt first
}

void AttackEditor::onMainWindowResize(int windowWidth, int windowHeight) {
	const float aiAreaWidth = windowWidth * 0.2f;
	aiPanel->setPosition(0, 0);
	aiID->setMaximumTextWidth(aiAreaWidth - GUI_PADDING_X * 2);
	aiPlayAttackAnimation->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	aiName->setSize(aiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	aiID->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	aiName->setPosition({ tgui::bindLeft(aiID), tgui::bindBottom(aiID) + GUI_PADDING_Y });
	aiPlayAttackAnimation->setPosition({ tgui::bindLeft(aiID), tgui::bindBottom(aiName) + GUI_PADDING_Y });
	aiPanel->setSize(aiAreaWidth, std::min(windowHeight*0.25f, aiPlayAttackAnimation->getPosition().y + aiPlayAttackAnimation->getSize().y + GUI_PADDING_Y));

	const float alAreaWidth = windowWidth * 0.2f;
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
	alList->onResize();

	const float emplAreaWidth = windowWidth * 0.25f;
	emplPanel->setPosition(tgui::bindRight(aiPanel), 0);
	emplPanel->setSize(emplAreaWidth, windowHeight);
	emplLabel->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	emplCreateEMP->setSize(100, TEXT_BUTTON_HEIGHT);
	emplDeleteEMP->setSize(100, TEXT_BUTTON_HEIGHT);
	emplTestEMP->setSize(100, TEXT_BUTTON_HEIGHT);
	emplTestEMP->setPosition(tgui::bindLeft(emplTree), windowHeight - emplCreateEMP->getSize().y - GUI_PADDING_Y);
	emplCreateEMP->setPosition(tgui::bindLeft(emplTree), tgui::bindTop(emplTestEMP) - emplCreateEMP->getSize().y - GUI_PADDING_Y);
	emplDeleteEMP->setPosition(tgui::bindRight(emplCreateEMP) + GUI_PADDING_X, tgui::bindTop(emplCreateEMP));
	emplTree->setPosition(tgui::bindLeft(emplLabel), tgui::bindBottom(emplLabel) + GUI_PADDING_Y);
	emplTree->setSize(emplAreaWidth - GUI_PADDING_X * 2, emplCreateEMP->getPosition().y - emplLabel->getPosition().y - emplLabel->getSize().y - GUI_PADDING_Y * 2);


	const float empiAreaWidth = windowWidth * 0.25f;
	empiIsBullet->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	empiHitboxRadius->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiDespawnTime->setSize(empiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empiActions->setSize(empiAreaWidth - GUI_PADDING_X * 2, 250);
	empiActions->onResize();
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
	empiAnimatable->setPosition(0, tgui::bindBottom(empiAnimatablePicture) + GUI_PADDING_Y);
	empiAnimatable->setSize(empiAreaWidth, 0); // height fixed by onContainerResize
	empiAnimatable->onContainerResize(empiAreaWidth, windowHeight);
	empiLoopAnimation->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	auto empiBaseSpritePicture = empiBaseSprite->getAnimatablePicture();
	empiBaseSpritePicture->setSize(100, 100);
	empiBaseSpritePicture->setPosition(tgui::bindLeft(empiBaseSpriteLabel), tgui::bindBottom(empiBaseSpriteLabel) + GUI_PADDING_Y);
	empiBaseSprite->setPosition(0, tgui::bindBottom(empiBaseSpritePicture) + GUI_PADDING_Y);
	empiBaseSprite->setSize(empiAreaWidth, 0); // height fixed by onContainerResize
	empiBaseSprite->onContainerResize(empiAreaWidth, windowHeight);
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
	empiLoopAnimationLabel->setPosition(tgui::bindLeft(empiAnimatableLabel), tgui::bindBottom(empiAnimatable) + GUI_PADDING_Y);
	empiLoopAnimation->setPosition(tgui::bindLeft(empiLoopAnimationLabel), tgui::bindBottom(empiLoopAnimationLabel) + GUI_PADDING_Y);
	empiBaseSpriteLabel->setPosition(tgui::bindLeft(empiLoopAnimation), tgui::bindBottom(empiLoopAnimation) + GUI_PADDING_Y);
	//empiBaseSprite->setPosition(tgui::bindLeft(empiBaseSpriteLabel), tgui::bindBottom(empiBaseSpriteLabel) + GUI_PADDING_Y);
	empiDamageLabel->setPosition(tgui::bindLeft(empiLoopAnimationLabel), tgui::bindBottom(empiBaseSprite) + GUI_PADDING_Y);
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


	const float empaiAreaWidth = windowWidth - (empiPanel->getPosition().x + empiAreaWidth);
	empaiPanel->setPosition(tgui::bindRight(empiPanel), 0);
	empaiPanel->setSize(empaiAreaWidth, windowHeight);
	empaiDuration->setSize(empaiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);
	empaiPolarDistance->setSize(empaiAreaWidth, 0);
	empaiPolarDistance->onContainerResize(empaiAreaWidth, windowHeight);
	empaiPolarAngle->setSize(empaiAreaWidth, 0);
	empaiPolarAngle->onContainerResize(empaiAreaWidth, windowHeight);
	empaiBezierControlPoints->setSize(empaiAreaWidth - GUI_PADDING_X * 2, 200);
	empaiBezierControlPoints->onResize();
	empaiAngleOffset->setSize(empaiAreaWidth, 0);
	empaiAngleOffset->onContainerResize(empaiAreaWidth, windowHeight);
	empaiHomingStrength->setSize(empaiAreaWidth, 0);
	empaiHomingStrength->onContainerResize(empaiAreaWidth, windowHeight);
	empaiHomingSpeed->setSize(empaiAreaWidth, 0);
	empaiHomingSpeed->onContainerResize(empaiAreaWidth, windowHeight);
	empaiEditBezierControlPoints->setSize(empaiAreaWidth - GUI_PADDING_X * 2, TEXT_BOX_HEIGHT);

	empaiInfo->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	empaiDurationLabel->setPosition(tgui::bindLeft(empaiInfo), tgui::bindBottom(empaiInfo) + GUI_PADDING_Y);
	empaiDuration->setPosition(GUI_PADDING_X, empaiDurationLabel->getPosition().y + empaiDurationLabel->getSize().y + GUI_PADDING_Y);
	empaiAngleOffsetLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiDuration) + GUI_PADDING_Y);
	empaiAngleOffset->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiAngleOffsetLabel));
	empaiPolarDistanceLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiAngleOffset) + GUI_PADDING_Y);
	empaiPolarDistance->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiPolarDistanceLabel));
	empaiPolarAngleLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiPolarDistance) + GUI_PADDING_Y);
	empaiPolarAngle->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiPolarAngleLabel));
	empaiBezierControlPointsLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiAngleOffset) + GUI_PADDING_Y);
	empaiBezierControlPoints->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiBezierControlPointsLabel));
	empaiEditBezierControlPoints->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiBezierControlPoints) + GUI_PADDING_Y);
	empaiHomingStrengthLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiDuration) + GUI_PADDING_Y);
	empaiHomingStrength->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiHomingStrengthLabel));
	empaiHomingSpeedLabel->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiHomingStrength) + GUI_PADDING_Y);
	empaiHomingSpeed->setPosition(GUI_PADDING_X, tgui::bindBottom(empaiHomingSpeedLabel));

	ibPanel->setPosition(0, 0);
	ibPanel->setSize("100%", "100%");
	ibText->setMaximumTextWidth(ibPanel->getSize().x - GUI_PADDING_X * 2);
	ibText->setPosition(GUI_PADDING_X, (ibPanel->getSize().y - ibText->getSize().y) / 2.0f);
}

void AttackEditor::reload() {
	empiSoundSettings->populateFileNames("Level Packs\\" + levelPack->getName() + "\\Sounds");
	
	empiBulletModel->removeAllItems();
	empiBulletModel->addItem("", "-1");
	for (auto it = levelPack->getBulletModelIteratorBegin(); it != levelPack->getBulletModelIteratorEnd(); it++) {
		empiBulletModel->addItem(it->second->getName(), std::to_string(it->second->getID()));
	}

	legalCheck();
}

std::vector<int> AttackEditor::legalCheck() {
	std::vector<int> illegals;

	std::string errorMessage = "";
	for (auto it = levelPack->getAttackIteratorBegin(); it != levelPack->getAttackIteratorEnd(); it++) {
		std::shared_ptr<EditorAttack> attack;
		if (unsavedAttacks.count(it->first) > 0) {
			attack = unsavedAttacks[it->first];
		} else {
			attack = it->second;
		}
		if (!attack->legal(*levelPack, *spriteLoader, errorMessage)) {
			illegals.push_back(attack->getID());
		}
	}

	//TODO: display errorMessage in an error log, a separate scrollpanel; error log pops out in bottom right and can be closed
	// error log should have a clear button and messages are timestamped

	return illegals;
}

void AttackEditor::selectAttack(int id) {
	deselectEMP();
	if (unsavedAttacks.count(id) > 0) {
		selectedAttack = unsavedAttacks[id];
	} else {
		// Make a copy of the attack in the LevelPack so that changes can be applied/discarded
		// whenever the user wants instead of modifying the LevelPack directly.
		selectedAttack = std::make_shared<EditorAttack>(levelPack->getAttack(id));
	}

	aiName->setReadOnly(false);
	alList->getListBox()->setSelectedItemById(std::to_string(id));
	buildEMPTree();
	alDeleteAttack->setEnabled(true);

	setAttackWidgetValues(selectedAttack, false);
}

void AttackEditor::deselectAttack() {
	deselectEMP();
	selectedAttack = nullptr;

	aiID->setText("No attack selected");
	aiName->setText("");
	aiName->setReadOnly(true);
	aiPlayAttackAnimation->setChecked(false);
	emplTree->removeAllItems();
	alList->getListBox()->deselectItem();
	alDeleteAttack->setEnabled(false);
}

void AttackEditor::selectEMP(std::shared_ptr<EditorAttack> parentAttack, int empID) {
	if (selectedAttack && selectedAttack->getID() != parentAttack->getID()) {
		selectAttack(parentAttack->getID());
	}
	deselectEMPA();
	
	// A copy of the EMP is not necessary because a copy of the selected attack has already been made
	// when selecting an attack.
	selectedEMP = selectedAttack->searchEMP(empID);

	emplCreateEMP->setEnabled(true);
	// Can't delete main EMP of an attack
	emplDeleteEMP->setEnabled(empID != selectedAttack->getMainEMP()->getID());
	emplTree->selectItem(selectedEMP->generatePathToThisEmp(getEMPTreeNodeText));
	// Can only test EMP if it's legal
	std::string useless;
	emplTestEMP->setEnabled(selectedEMP->legal(*levelPack, *spriteLoader, useless));

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

	setEMPWidgetValues(selectedEMP, selectedAttack, true);
}

void AttackEditor::deselectEMP() {
	deselectEMPA();
	selectedEMP = nullptr;

	emplCreateEMP->setEnabled(false);
	emplDeleteEMP->setEnabled(false);
	emplTestEMP->setEnabled(false);

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

	empaiDurationLabel->setVisible(true);
	empaiDuration->setVisible(true);
	if (dynamic_cast<MoveCustomPolarEMPA*>(selectedEMPA.get())) {
		MoveCustomPolarEMPA* concreteEMPA = dynamic_cast<MoveCustomPolarEMPA*>(selectedEMPA.get());
		empaiPolarDistance->setVisible(true);
		empaiPolarDistance->setTFV(concreteEMPA->getDistance(), selectedEMPA->getTime(), "distance");
		empaiPolarAngle->setVisible(true);
		empaiPolarAngle->setTFV(concreteEMPA->getAngle(), selectedEMPA->getTime(), "angle");
		empaiBezierControlPoints->setVisible(false);
		empaiEditBezierControlPoints->setVisible(false);
		empaiAngleOffset->setVisible(true);
		empaiHomingStrength->setVisible(false);
		empaiHomingSpeed->setVisible(false);
		empaiInfo->setText("Attack ID: " + tos(selectedAttack->getID()) + "\nMovable point ID: " + tos(selectedEMP->getID())
			+ "\nPolar coordinates movement action");
	} else if (dynamic_cast<MoveCustomBezierEMPA*>(selectedEMPA.get())) {
		empaiPolarDistance->setVisible(false);
		empaiPolarAngle->setVisible(false);
		empaiBezierControlPoints->setVisible(true);
		empaiEditBezierControlPoints->setVisible(true);
		empaiAngleOffset->setVisible(true);
		empaiHomingStrength->setVisible(false);
		empaiHomingSpeed->setVisible(false);
		empaiInfo->setText("Attack ID: " + tos(selectedAttack->getID()) + "\nMovable point ID: " + tos(selectedEMP->getID())
			+ "\nBezier movement action");
		populateEmpaiBezierControlPoints(selectedEMPA);
	} else if (dynamic_cast<MovePlayerHomingEMPA*>(selectedEMPA.get())) {
		MovePlayerHomingEMPA* concreteEMPA = dynamic_cast<MovePlayerHomingEMPA*>(selectedEMPA.get());
		empaiPolarDistance->setVisible(false);
		empaiPolarAngle->setVisible(false);
		empaiBezierControlPoints->setVisible(false);
		empaiEditBezierControlPoints->setVisible(false);
		empaiAngleOffset->setVisible(false);
		empaiHomingStrength->setVisible(true);
		empaiHomingStrength->setTFV(concreteEMPA->getHomingStrength(), selectedEMPA->getTime(), "homingStrength");
		empaiHomingSpeed->setVisible(true);
		empaiHomingSpeed->setTFV(concreteEMPA->getSpeed(), selectedEMPA->getTime(), "speed");
		empaiInfo->setText("Attack ID: " + tos(selectedAttack->getID()) + "\nMovable point ID: " + tos(selectedEMP->getID())
			+ "\nHoming movement action");
	} else if (dynamic_cast<StayStillAtLastPositionEMPA*>(selectedEMPA.get())) {
		empaiPolarDistance->setVisible(false);
		empaiPolarAngle->setVisible(false);
		empaiBezierControlPoints->setVisible(false);
		empaiEditBezierControlPoints->setVisible(false);
		empaiAngleOffset->setVisible(false);
		empaiHomingStrength->setVisible(false);
		empaiHomingSpeed->setVisible(false);
		empaiInfo->setText("Attack ID: " + tos(selectedAttack->getID()) + "\nMovable point ID: " + tos(selectedEMP->getID())
			+ "\nStay still action");
	} else if (dynamic_cast<DetachFromParentEMPA*>(selectedEMPA.get())) {
		empaiPolarDistance->setVisible(false);
		empaiPolarAngle->setVisible(false);
		empaiBezierControlPoints->setVisible(false);
		empaiEditBezierControlPoints->setVisible(false);
		empaiAngleOffset->setVisible(false);
		empaiHomingStrength->setVisible(false);
		empaiHomingSpeed->setVisible(false);
		empaiInfo->setText("Attack ID: " + tos(selectedAttack->getID()) + "\nMovable point ID: " + tos(selectedEMP->getID())
			+ "\Detach action");
	} else {
		// This means you forgot to add a case
		assert(false);
	}
	//TODO: add EMPAs to this if any other EMPAs also use TFVs
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

	for (auto w : empaiPanel->getWidgets()) {
		w->setVisible(false);
	}
	empaiInfo->setVisible(true);
	empaiInfo->setText("No Movable point action selected");
}

void AttackEditor::setEMPWidgetValues(std::shared_ptr<EditorMovablePoint> emp, std::shared_ptr<EditorAttack> parentAttack, bool fromInit) {
	if (!fromInit) {
		setAttackWidgetValues(parentAttack, true);
	}
	if (emp != selectedEMP) {
		if (parentAttack != selectedAttack) {
			selectAttack(parentAttack->getID());
		}
		selectEMP(parentAttack, emp->getID());
	}

	if (!fromInit) {
		buildEMPTree();
		selectEMP(parentAttack, emp->getID());
	}

	ignoreSignal = true;
	// Can only test EMP if it's legal
	std::string useless;
	emplTestEMP->setEnabled(selectedEMP->legal(*levelPack, *spriteLoader, useless));

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
		empiBulletModel->deselectItem();
	} else {
		empiBulletModel->setSelectedItemById(std::to_string(emp->getBulletModelID()));
	}

	// Can't change spawn time of main EMP
	bool isBullet = emp->getIsBullet();
	bool usesModel = emp->usesBulletModel();
	empiSpawnTypeTime->setEnabled(parentAttack->getMainEMP()->getID() != emp->getID());
	empiShadowTrailLifespanLabel->setEnabled(isBullet);
	empiShadowTrailLifespan->setEnabled(isBullet);
	empiShadowTrailIntervalLabel->setEnabled(isBullet);
	empiShadowTrailInterval->setEnabled(isBullet);
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
	empiInheritRadius->setEnabled(isBullet);
	empiInheritDamage->setEnabled(isBullet);
	empiAnimatable->setEnabled(!usesModel || !emp->getInheritAnimatables());
	empiLoopAnimation->setEnabled((!usesModel || !emp->getInheritAnimatables()) && (empiAnimatable->getValue().getAnimatableName() != "" && !empiAnimatable->getValue().isSprite()));
	empiBaseSprite->setEnabled((!usesModel || !emp->getInheritAnimatables()) && (empiAnimatable->getValue().getAnimatableName() != "" && !empiAnimatable->getValue().isSprite() && !empiLoopAnimation->isChecked()));
	empiDamage->setEnabled(!usesModel || !emp->getInheritDamage());
	empiDespawnTime->setEnabled(!usesModel || !emp->getInheritDespawnTime());
	empiHitboxRadius->setEnabled(isBullet && (!usesModel || !emp->getInheritRadius()));
	empiShadowTrailInterval->setEnabled(!usesModel || !emp->getInheritShadowTrailInterval());
	empiShadowTrailLifespan->setEnabled(!usesModel || !emp->getInheritShadowTrailLifespan());
	empiSoundSettings->setEnabled(!usesModel || !emp->getInheritSoundSettings());
	empiPierceResetTime->setEnabled(empiOnCollisionAction->getSelectedItemId() == getID(PIERCE_ENTITY));

	empiInheritAnimatables->setChecked(emp->getInheritAnimatables());
	empiInheritDamage->setChecked(emp->getInheritDamage());
	empiInheritDespawnTime->setChecked(emp->getInheritDespawnTime());
	empiInheritRadius->setChecked(emp->getInheritRadius());
	empiInheritShadowTrailInterval->setChecked(emp->getInheritShadowTrailInterval());
	empiInheritShadowTrailLifespan->setChecked(emp->getInheritShadowTrailLifespan());
	empiInheritSoundSettings->setChecked(emp->getInheritSoundSettings());

	if (emp->usesBulletModel()) {
		empiBulletModel->setSelectedItemById(std::to_string(emp->getBulletModelID()));

		std::shared_ptr<BulletModel> model = levelPack->getBulletModel(emp->getBulletModelID());
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

	ignoreSignal = false;
}

void AttackEditor::createAttack() {
	std::shared_ptr<EditorAttack> attack = levelPack->createAttack();
	buildAttackList(true);
}

void AttackEditor::deleteAttack(std::shared_ptr<EditorAttack> attack, bool autoScrollAttackListToBottom) {
	//TODO: prompt: are you sure? this attack may be in use by some attack patterns.

	if (selectedAttack && selectedAttack->getID() == attack->getID()) {
		deselectAttack();
	}
	alList->getListBox()->removeItemById(std::to_string(attack->getID()));
	alList->onListBoxItemsChange();

	if (unsavedAttacks.count(attack->getID()) > 0) {
		unsavedAttacks.erase(attack->getID());
	}

	levelPack->deleteAttack(attack->getID());
	buildAttackList(autoScrollAttackListToBottom);
}

void AttackEditor::saveAttack(std::shared_ptr<EditorAttack> attack) {
	levelPack->updateAttack(attack);

	int id = attack->getID();
	if (alList->getListBox()->containsId(std::to_string(id))) {
		alList->getListBox()->changeItemById(std::to_string(id), attack->getName() + " [id=" + std::to_string(id) + "]");
	}
	alList->onListBoxItemsChange();
	if (unsavedAttacks.count(id) > 0) {
		unsavedAttacks.erase(id);
	}

	// Reselect attack so that selectedAttack isn't pointing to the same EditorAttack as the one in levelPack and is pointing to a copy of levelPack's EditorAttack of the same ID.
	// The "levelPack->updateAttack(attack);" line causes selectedAttack, if (selectedAttack == attack), to point to the same EditorAttack as the one in levelPack.
	if (selectedAttack && selectedAttack == attack) {
		auto prevSelectedEMP = selectedEMP;
		auto prevSelectedEMPAIndex = selectedEMPAIndex;
		selectAttack(selectedAttack->getID());
		// This is to reselect things that might have been deselected by selectAttack()
		if (prevSelectedEMP) {
			selectEMP(selectedAttack, prevSelectedEMP->getID());
			if (prevSelectedEMPAIndex != -1) {
				selectEMPA(prevSelectedEMPAIndex);
			}
		}
	}

	//TODO: legal check on this attack only
}

void AttackEditor::discardAttackChanges(std::shared_ptr<EditorAttack> attack) {
	unsavedAttacks.erase(attack->getID());

	std::shared_ptr<const EditorAttack> revertedAttack = levelPack->getAttack(attack->getID());

	int id = attack->getID();
	if (alList->getListBox()->containsId(std::to_string(id))) {
		alList->getListBox()->changeItemById(std::to_string(id), revertedAttack->getName() + " [id=" + std::to_string(id) + "]");
	}
	alList->onListBoxItemsChange();
	if (selectedAttack && selectedAttack->getID() == id) {
		deselectAttack();
	}
}

std::shared_ptr<EditorMovablePoint> AttackEditor::createEMP(std::shared_ptr<EditorAttack> empOwner, std::shared_ptr<EditorMovablePoint> parent, bool addEMPToParentChildrenList) {
	std::shared_ptr<EditorMovablePoint> emp = parent->createChild(addEMPToParentChildrenList);
	
	emplTree->addItem(emp->generatePathToThisEmp(&AttackEditor::getEMPTreeNodeText));

	setAttackWidgetValues(empOwner, true);

	return emp;
}

void AttackEditor::deleteEMP(std::shared_ptr<EditorAttack> empOwner, std::shared_ptr<EditorMovablePoint> emp) {
	assert(emp->getID() != empOwner->getMainEMP()->getID());

	emp->detachFromParent();
	buildEMPTree();
}

void AttackEditor::deleteEMPA(std::shared_ptr<EditorMovablePoint> emp, int empaIndex) {
	if (selectedEMP && emp == selectedEMP && selectedEMPAIndex == empaIndex) {
		deselectEMPA();
	}
	
	emp->removeAction(empaIndex);
	buildEMPIActions();
}

void AttackEditor::setAttackWidgetValues(std::shared_ptr<EditorAttack> attackWithUnsavedChanges, bool attackWasModified) {
	if (attackWithUnsavedChanges != selectedAttack) {
		selectAttack(attackWithUnsavedChanges->getID());
	}

	int id = attackWithUnsavedChanges->getID();
	if (attackWasModified) {
		unsavedAttacks[id] = attackWithUnsavedChanges;
	}
	// Add asterisk to the entry in attack list
	if (attackWasModified && unsavedAttacks.count(id) > 0) {
		alList->getListBox()->changeItemById(std::to_string(id), "*" + attackWithUnsavedChanges->getName() + " [id=" + std::to_string(id) + "]");
	}
	alList->onListBoxItemsChange();

	aiID->setText("Attack ID: " + std::to_string(id));
	aiName->setText(selectedAttack->getName());
	aiPlayAttackAnimation->setChecked(selectedAttack->getPlayAttackAnimation());
}

void AttackEditor::setEMPAWidgetValues(std::shared_ptr<EMPAction> empa, std::shared_ptr<EditorMovablePoint> parentEMP, std::shared_ptr<EditorAttack> parentAttack) {
	if (parentEMP != selectedEMP) {
		if (parentAttack != selectedAttack) {
			selectAttack(parentAttack->getID());
		}
		selectEMP(parentAttack, parentEMP->getID());
	}
	if (empa != selectedEMPA) {
		int i = 0;
		for (auto e : selectedEMP->getActions()) {
			if (e == empa) {
				selectEMPA(i);
				break;
			}
			i++;
		}
	} else {
		selectEMPA(selectedEMPAIndex);
	}

	// Since EditorUtilities::TFVGroup already takes care of undos, there is no need to set those widgets' values here

	if (dynamic_cast<MoveCustomBezierEMPA*>(empa.get())) {
		populateEmpaiBezierControlPoints(selectedEMPA);
	}
	empaiDuration->setValue(empa->getTime());

	// This function is called only when a change is made to an EMPA and therefore also to the parent EditorAttack as well
	int id = parentAttack->getID();
	unsavedAttacks[id] = parentAttack;
	// Add asterisk to the entry in attack list
	alList->getListBox()->changeItemById(std::to_string(id), "*" + parentAttack->getName() + " [id=" + std::to_string(id) + "]");
	alList->onListBoxItemsChange();
}

void AttackEditor::buildEMPTree() {
	emplTree->removeAllItems();
	for (std::vector<sf::String> item : selectedAttack->generateTreeViewEmpHierarchy(&AttackEditor::getEMPTreeNodeText)) {
		emplTree->addItem(item);
	}

	emplTree->deselectItem();
	if (selectedEMP) {
		emplTree->selectItem(selectedEMP->generatePathToThisEmp(getEMPTreeNodeText));
	}
}

sf::String AttackEditor::getEMPTreeNodeText(const EditorMovablePoint& emp) {
	//TODO: inidicate if it's a bullet and other stuff
	// Whether EMP is a bullet or just an anchor (a reference point; not a bullet)
	std::string bulletStr = emp.getIsBullet() ? "[B]" : "[A]";
	std::string idStr = "[id=" + std::to_string(emp.getID()) + "]";
	return bulletStr + " " + idStr;
}

void AttackEditor::buildEMPIActions() {
	empiActions->getListBox()->removeAllItems();
	auto actions = selectedEMP->getActions();
	float curTime = 0;
	for (int i = 0; i < actions.size(); i++) {
		empiActions->getListBox()->addItem("[t=" + formatNum(curTime) + "] " + actions[i]->getGuiFormat(), std::to_string(i));
	}
	empiActions->onListBoxItemsChange();
}

void AttackEditor::buildAttackList(bool autoscrollToBottom) {
	alList->getListBox()->removeAllItems();
	if (!autoscrollToBottom) {
		alList->getListBox()->setAutoScroll(false);
	}
	for (auto it = levelPack->getAttackIteratorBegin(); it != levelPack->getAttackIteratorEnd(); it++) {
		if (unsavedAttacks.count(it->second->getID()) > 0) {
			alList->getListBox()->addItem("*" + it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
		} else {
			alList->getListBox()->addItem(it->second->getName() + " [id=" + std::to_string(it->second->getID()) + "]", std::to_string(it->second->getID()));
		}
	}
	alList->getListBox()->setAutoScroll(true);
	alList->onListBoxItemsChange();
}

void AttackEditor::onMainWindowRender(float deltaTime) {
	std::lock_guard<std::recursive_mutex> lock(*tguiMutex);

	empiAnimatable->getAnimatablePicture()->update(deltaTime);
	empiBaseSprite->getAnimatablePicture()->update(deltaTime);
}

void AttackEditor::onGameplayTestWindowClose() {
	if (editingEMPABezierControlPoints) {
		onGameplayTestWindowBezierControlPointsEditingEnd(false, std::vector<sf::Vector2f>());
	}
}

void AttackEditor::onGameplayTestWindowBezierControlPointsEditingEnd(bool changesWereMade, std::vector<sf::Vector2f> newControlPoints) {
	// Stop blocking input
	ibPanel->setVisible(false);

	editingEMPABezierControlPoints = false;

	if (changesWereMade) {
		MoveCustomBezierEMPA* bezierEMPA = dynamic_cast<MoveCustomBezierEMPA*>(selectedEMPA.get());
		std::vector<sf::Vector2f> oldControlPoints = bezierEMPA->getUnrotatedControlPoints();
		mainWindowUndoStack.execute(UndoableCommand(
			[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, &bezierEMPA = bezierEMPA, &selectedEMPA = selectedEMPA, newControlPoints]() {
			bezierEMPA->setUnrotatedControlPoints(newControlPoints);
			setEMPAWidgetValues(selectedEMPA, selectedEMP, selectedAttack);
		},
			[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, &bezierEMPA = bezierEMPA, &selectedEMPA = selectedEMPA, oldControlPoints]() {
			bezierEMPA->setUnrotatedControlPoints(oldControlPoints);
			setEMPAWidgetValues(selectedEMPA, selectedEMP, selectedAttack);
		}));
	}
}

void AttackEditor::onEmpiHitboxRadiusChange(float value) {
	if (ignoreSignal) return;
	float oldValue = selectedEMP->getHitboxRadius();
	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, value]() {
		selectedEMP->setHitboxRadius(value);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	},
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, oldValue]() {
		selectedEMP->setHitboxRadius(oldValue);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	}));
}

void AttackEditor::onEmpiDespawnTimeChange(float value) {
	if (ignoreSignal) return;
	float oldValue = selectedEMP->getDespawnTime();
	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, value]() {
		selectedEMP->setDespawnTime(value);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	},
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, oldValue]() {
		selectedEMP->setDespawnTime(oldValue);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	}));
}

void AttackEditor::onEmpiSpawnTypeTimeChange(float value) {
	if (ignoreSignal) return;
	float oldValue = selectedEMP->getSpawnType()->getTime();
	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, value]() {
		selectedEMP->getSpawnType()->setTime(value);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	},
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, oldValue]() {
		selectedEMP->getSpawnType()->setTime(oldValue);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	}));
}

void AttackEditor::onEmpiSpawnTypeXChange(float value) {
	if (ignoreSignal) return;
	float oldValue = selectedEMP->getSpawnType()->getX();
	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, value]() {
		selectedEMP->getSpawnType()->setX(value);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	},
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, oldValue]() {
		selectedEMP->getSpawnType()->setX(oldValue);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	}));
}

void AttackEditor::onEmpiSpawnTypeYChange(float value) {
	if (ignoreSignal) return;
	float oldValue = selectedEMP->getSpawnType()->getY();
	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, value]() {
		selectedEMP->getSpawnType()->setY(value);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	},
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, oldValue]() {
		selectedEMP->getSpawnType()->setY(oldValue);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	}));
}

void AttackEditor::onEmpiShadowTrailLifespanChange(float value) {
	if (ignoreSignal) return;
	float oldValue = selectedEMP->getShadowTrailLifespan();
	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, value]() {
		selectedEMP->setShadowTrailLifespan(value);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	},
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, oldValue]() {
		selectedEMP->setShadowTrailLifespan(oldValue);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	}));
}

void AttackEditor::onEmpiShadowTrailIntervalChange(float value) {
	if (ignoreSignal) return;
	float oldValue = selectedEMP->getShadowTrailInterval();
	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, value]() {
		selectedEMP->setShadowTrailInterval(value);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	},
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, oldValue]() {
		selectedEMP->setShadowTrailInterval(oldValue);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	}));
}

void AttackEditor::onAnimatableChange(Animatable value) {
	if (ignoreSignal) return;
	Animatable oldValue = selectedEMP->getAnimatable();
	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, value]() {
		selectedEMP->setAnimatable(value);
		// Not setEMPWidgetValues() because of some random infinite loop bug that I can't figure out how to fix
		// so this is some dumb hack
		this->ignoreSignal = true;
		empiAnimatable->setSelectedItem(selectedEMP->getAnimatable());
		setAttackWidgetValues(selectedAttack, true);
		this->ignoreSignal = false;
	},
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, oldValue]() {
		selectedEMP->setAnimatable(oldValue);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	}));
}

void AttackEditor::onBaseSpriteChange(Animatable value) {
	if (ignoreSignal) return;
	Animatable oldValue = selectedEMP->getBaseSprite();
	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, value]() {
		selectedEMP->setBaseSprite(value);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	},
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, oldValue]() {
		selectedEMP->setBaseSprite(oldValue);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	}));
}

void AttackEditor::onEmpiDamageChange(float value) {
	if (ignoreSignal) return;
	float oldValue = selectedEMP->getDamage();
	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, value]() {
		selectedEMP->setDamage(value);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	},
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, oldValue]() {
		selectedEMP->setDamage(oldValue);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	}));
}

void AttackEditor::onEmpiSoundSettingsChange(SoundSettings value) {
	if (ignoreSignal) return;
	SoundSettings oldValue = selectedEMP->getSoundSettings();
	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, value]() {
		selectedEMP->setSoundSettings(value);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	},
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, oldValue]() {
		selectedEMP->setSoundSettings(oldValue);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	}));
}

void AttackEditor::onEmpiPierceResetTimeChange(float value) {
	if (ignoreSignal) return;
	float oldValue = selectedEMP->getPierceResetTime();
	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, value]() {
		selectedEMP->setPierceResetTime(value);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	},
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, oldValue]() {
		selectedEMP->setPierceResetTime(oldValue);
		setEMPWidgetValues(selectedEMP, selectedAttack, false);
	}));
}

void AttackEditorMainWindow::handleEvent(sf::Event event) {
	UndoableEditorWindow::handleEvent(event);
}

void AttackEditor::populateEmpaiBezierControlPoints(std::shared_ptr<EMPAction> empa) {
	empaiBezierControlPoints->getListBox()->removeAllItems();
	MoveCustomBezierEMPA* bezier = dynamic_cast<MoveCustomBezierEMPA*>(empa.get());
	int i = 0;
	for (sf::Vector2f cp : bezier->getUnrotatedControlPoints()) {
		empaiBezierControlPoints->getListBox()->addItem("[" + std::to_string(i + 1) + "] x=" + formatNum(cp.x) +
			" y=" + formatNum(cp.y) , std::to_string(i));
		i++;
	}
	empaiBezierControlPoints->onListBoxItemsChange();
}

void AttackEditor::onEmpaiDurationChange(float value) {
	if (ignoreSignal) return;
	float oldValue = selectedEMPA->getTime();
	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMPA = this->selectedEMPA, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, value]() {
		selectedEMPA->setTime(value);

		if (dynamic_cast<MoveCustomPolarEMPA*>(selectedEMPA.get()) != nullptr) {
			auto ptr = dynamic_cast<MoveCustomPolarEMPA*>(selectedEMPA.get());
			ptr->getDistance()->setMaxTime(value);
			ptr->getAngle()->setMaxTime(value);
		} else if (dynamic_cast<MovePlayerHomingEMPA*>(selectedEMPA.get()) != nullptr) {
			auto ptr = dynamic_cast<MovePlayerHomingEMPA*>(selectedEMPA.get());
			ptr->getHomingStrength()->setMaxTime(value);
			ptr->getSpeed()->setMaxTime(value);
		}
		//TODO: add EMPAs to this if any other EMPAs also use TFVs

		setEMPAWidgetValues(selectedEMPA, selectedEMP, selectedAttack);
	},
		[this, &selectedEMPA = this->selectedEMPA, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, oldValue]() {
		selectedEMPA->setTime(oldValue);

		if (dynamic_cast<MoveCustomPolarEMPA*>(selectedEMPA.get()) != nullptr) {
			auto ptr = dynamic_cast<MoveCustomPolarEMPA*>(selectedEMPA.get());
			ptr->getDistance()->setMaxTime(oldValue);
			ptr->getAngle()->setMaxTime(oldValue);
		} else if (dynamic_cast<MovePlayerHomingEMPA*>(selectedEMPA.get()) != nullptr) {
			auto ptr = dynamic_cast<MovePlayerHomingEMPA*>(selectedEMPA.get());
			ptr->getHomingStrength()->setMaxTime(oldValue);
			ptr->getSpeed()->setMaxTime(oldValue);
		}
		//TODO: add EMPAs to this if any other EMPAs also use TFVs

		setEMPAWidgetValues(selectedEMPA, selectedEMP, selectedAttack);
	}));
}

void AttackEditor::onEmpaiAngleOffsetChange(std::shared_ptr<EMPAAngleOffset> oldOffset, std::shared_ptr<EMPAAngleOffset> updatedOffset) {
	std::shared_ptr<EMPAAngleOffset> copyOfOld = oldOffset->clone();

	mainWindowUndoStack.execute(UndoableCommand(
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, &selectedEMPA = selectedEMPA, &oldOffset = oldOffset, updatedOffset]() {
		*oldOffset = *updatedOffset;
		setEMPAWidgetValues(selectedEMPA, selectedEMP, selectedAttack);
	},
		[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, &selectedEMPA = selectedEMPA, &oldOffset = oldOffset, copyOfOld]() {
		*oldOffset = *copyOfOld;
		setEMPAWidgetValues(selectedEMPA, selectedEMP, selectedAttack);
	}));
}

void AttackEditor::onTFVEditingStart() {
	// Reselect EMPA to call the relevant TFVGroup::setTFV()'s again
	selectEMPA(selectedEMPAIndex);
	// Block all user input
	ibPanel->setVisible(true);
	ibText->setText("Currently editing a TFV for Attack ID " + std::to_string(selectedAttack->getID()) + ", EMP ID " +
		std::to_string(selectedEMP->getID()) + ", EMPA #" + std::to_string(selectedEMPAIndex + 1) + " in the gameplay test window.");
}

void AttackEditor::onTFVEditingEnd(std::shared_ptr<TFV> oldTFV, std::shared_ptr<TFV> updatedTFV, std::string tfvIdentifier, bool saveChanges) {
	// Stop blocking input
	ibPanel->setVisible(false);

	if (saveChanges) {
		onTFVEditingSave(oldTFV, updatedTFV, tfvIdentifier);
	}
}

void AttackEditor::onTFVEditingSave(std::shared_ptr<TFV> oldTFV, std::shared_ptr<TFV> updatedTFV, std::string tfvIdentifier) {
	std::shared_ptr<TFV> copyOfOld = oldTFV->clone();
	
	if (dynamic_cast<MoveCustomPolarEMPA*>(selectedEMPA.get()) != nullptr) {
		mainWindowUndoStack.execute(UndoableCommand(
			[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, &selectedEMPA = selectedEMPA, oldTFV, updatedTFV, tfvIdentifier]() {
			if (tfvIdentifier == "distance") {
				dynamic_cast<MoveCustomPolarEMPA*>(selectedEMPA.get())->setDistance(updatedTFV);
			} else {
				dynamic_cast<MoveCustomPolarEMPA*>(selectedEMPA.get())->setAngle(updatedTFV);
			}
			setEMPAWidgetValues(selectedEMPA, selectedEMP, selectedAttack);
		},
			[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, &selectedEMPA = selectedEMPA, oldTFV, copyOfOld, tfvIdentifier]() {
			if (tfvIdentifier == "distance") {
				dynamic_cast<MoveCustomPolarEMPA*>(selectedEMPA.get())->setDistance(copyOfOld);
			} else {
				dynamic_cast<MoveCustomPolarEMPA*>(selectedEMPA.get())->setAngle(copyOfOld);
			}
			setEMPAWidgetValues(selectedEMPA, selectedEMP, selectedAttack);
		}));
	} else if (dynamic_cast<MovePlayerHomingEMPA*>(selectedEMPA.get()) != nullptr) {
		mainWindowUndoStack.execute(UndoableCommand(
			[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, &selectedEMPA = selectedEMPA, oldTFV, updatedTFV, tfvIdentifier]() {
			if (tfvIdentifier == "homingStrength") {
				dynamic_cast<MovePlayerHomingEMPA*>(selectedEMPA.get())->setHomingStrength(updatedTFV);
			} else {
				dynamic_cast<MovePlayerHomingEMPA*>(selectedEMPA.get())->setSpeed(updatedTFV);
			}
			setEMPAWidgetValues(selectedEMPA, selectedEMP, selectedAttack);
		},
			[this, &selectedEMP = this->selectedEMP, &selectedAttack = this->selectedAttack, &selectedEMPA = selectedEMPA, oldTFV, copyOfOld, tfvIdentifier]() {
			if (tfvIdentifier == "homingStrength") {
				dynamic_cast<MovePlayerHomingEMPA*>(selectedEMPA.get())->setHomingStrength(copyOfOld);
			} else {
				dynamic_cast<MovePlayerHomingEMPA*>(selectedEMPA.get())->setSpeed(copyOfOld);
			}
			setEMPAWidgetValues(selectedEMPA, selectedEMP, selectedAttack);
		}));
	}
	//TODO: add EMPAs to this if any other EMPAs also use TFVs
}
