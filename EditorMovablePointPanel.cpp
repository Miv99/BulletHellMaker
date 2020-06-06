#include "EditorMovablePointPanel.h"
#include "EditorMovablePointActionPanel.h"

const std::string EditorMovablePointPanel::PROPERTIES_TAB_NAME = "MP Properties";
const std::string EditorMovablePointPanel::EMPA_TAB_NAME_FORMAT = "Action %d";
const int EditorMovablePointPanel::EMPA_TAB_NAME_FORMAT_NUMBER_INDEX = 7;

std::string EditorMovablePointPanel::getID(BULLET_ON_COLLISION_ACTION onCollisionAction) {
	return std::to_string(static_cast<int>(onCollisionAction));
}

BULLET_ON_COLLISION_ACTION EditorMovablePointPanel::fromID(std::string id) {
	return static_cast<BULLET_ON_COLLISION_ACTION>(std::stoi(std::string(id)));
}

std::string EditorMovablePointPanel::getID(std::shared_ptr<EMPSpawnType> spawnType) {
	if (dynamic_cast<SpecificGlobalEMPSpawn*>(spawnType.get())) {
		return "0";
	} else if (dynamic_cast<EntityRelativeEMPSpawn*>(spawnType.get())) {
		return "1";
	} else if (dynamic_cast<EntityAttachedEMPSpawn*>(spawnType.get())) {
		return "2";
	}
}

EditorMovablePointPanel::EditorMovablePointPanel(MainEditorWindow & mainEditorWindow, LevelPack & levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorMovablePoint> emp, int undoStackSize)
	: CopyPasteable("EditorMovablePoint"), mainEditorWindow(mainEditorWindow), levelPack(levelPack), emp(emp), clipboard(clipboard), undoStack(UndoStack(undoStackSize)) {
	spawnTypePositionMarkerPlacer = SingleMarkerPlacer::create(*(mainEditorWindow.getWindow()));
	spawnTypePositionMarkerPlacer->setPosition(0, 0);
	spawnTypePositionMarkerPlacer->setSize("100%", "100%");
	spawnTypePositionMarkerPlacerFinishEditing = tgui::Button::create();
	spawnTypePositionMarkerPlacerFinishEditing->setSize(100, TEXT_BUTTON_HEIGHT);
	spawnTypePositionMarkerPlacerFinishEditing->setTextSize(TEXT_SIZE);
	spawnTypePositionMarkerPlacerFinishEditing->setText("Finish");
	spawnTypePositionMarkerPlacerFinishEditing->connect("Pressed", [&]() {
		finishEditingSpawnTypePosition();
	});
	
	tabs = TabsWithPanel::create(mainEditorWindow);
	tabs->setPosition(0, 0);
	tabs->setSize("100%", "100%");
	add(tabs);

	{
		// Properties

		properties = tgui::ScrollablePanel::create();
		
		id = tgui::Label::create();
		empiAnimatableLabel = tgui::Label::create();
		empiAnimatable = AnimatableChooser::create(spriteLoader);
		
		// Invisible if empiAnimatable's value is a sprite
		empiLoopAnimation = tgui::CheckBox::create("Loop animation");
		
		// Invisible if loopAnimation is checked or a sprite is selected in empiAnimatable
		empiBaseSpriteLabel = tgui::Label::create();
		empiBaseSprite = AnimatableChooser::create(spriteLoader, true);
		
		isBullet = tgui::CheckBox::create("Is bullet");
		empiHitboxRadiusLabel = tgui::Label::create();
		empiHitboxRadius = NumericalEditBoxWithLimits::create();
		
		empiActionsLabel = tgui::Label::create();
		// Entry ID is index in list of the EMP's actions
		empiActions = ListBoxScrollablePanel::create();
		
		empiActionsAdd = tgui::Button::create();
		empiActionsDelete = tgui::Button::create();

		empiDespawnTimeLabel = tgui::Label::create();
		// Max value is sum of time taken for every EMPA in empiActions
		empiDespawnTime = std::make_shared<SliderWithEditBox>();

		empiSpawnTypeLabel = tgui::Label::create();
		// Entry ID is from getID()
		empiSpawnType = tgui::ComboBox::create();
		empiSpawnTypeTimeLabel = tgui::Label::create();
		empiSpawnTypeTime = NumericalEditBoxWithLimits::create();
		empiSpawnTypeXLabel = tgui::Label::create();
		empiSpawnTypeX = NumericalEditBoxWithLimits::create();
		empiSpawnTypeYLabel = tgui::Label::create();
		empiSpawnTypeY = NumericalEditBoxWithLimits::create();
		empiSpawnLocationManualSet = tgui::Button::create();

		empiShadowTrailLifespanLabel = tgui::Label::create();
		empiShadowTrailLifespan = NumericalEditBoxWithLimits::create();
		empiShadowTrailIntervalLabel = tgui::Label::create();
		empiShadowTrailInterval = NumericalEditBoxWithLimits::create();

		empiDamageLabel = tgui::Label::create();
		empiDamage = NumericalEditBoxWithLimits::create();

		empiOnCollisionActionLabel = tgui::Label::create();
		// Entry ID obtained from getID()
		empiOnCollisionAction = tgui::ComboBox::create();

		empiPierceResetTimeLabel = tgui::Label::create();
		empiPierceResetTime = NumericalEditBoxWithLimits::create();

		empiSoundSettings = SoundSettingsGroup::create(format(LEVEL_PACK_SOUND_FOLDER_PATH, levelPack.getName().c_str()));

		empiBulletModelLabel = tgui::Label::create();
		// Entry ID is bullet model ID
		empiBulletModel = tgui::ComboBox::create();

		empiInheritRadius = tgui::CheckBox::create();
		empiInheritDespawnTime = tgui::CheckBox::create();
		empiInheritShadowTrailInterval = tgui::CheckBox::create();
		empiInheritShadowTrailLifespan = tgui::CheckBox::create();
		empiInheritAnimatables = tgui::CheckBox::create();
		empiInheritDamage = tgui::CheckBox::create();
		empiInheritSoundSettings = tgui::CheckBox::create();

		properties->setHorizontalScrollAmount(SCROLL_AMOUNT);
		properties->setVerticalScrollAmount(SCROLL_AMOUNT);

		empiSpawnType->setChangeItemOnScroll(false);
		empiOnCollisionAction->setChangeItemOnScroll(false);
		empiBulletModel->setChangeItemOnScroll(false);

		id->setTextSize(TEXT_SIZE);
		empiAnimatableLabel->setTextSize(TEXT_SIZE);
		empiLoopAnimation->setTextSize(TEXT_SIZE);
		empiBaseSpriteLabel->setTextSize(TEXT_SIZE);
		isBullet->setTextSize(TEXT_SIZE);
		empiHitboxRadiusLabel->setTextSize(TEXT_SIZE);
		empiHitboxRadius->setTextSize(TEXT_SIZE);
		empiActionsLabel->setTextSize(TEXT_SIZE);
		empiActions->setTextSize(TEXT_SIZE);
		empiActionsAdd->setTextSize(TEXT_SIZE);
		empiActionsDelete->setTextSize(TEXT_SIZE);
		empiDespawnTimeLabel->setTextSize(TEXT_SIZE);
		empiDespawnTime->setTextSize(TEXT_SIZE);
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
		empiShadowTrailLifespan->setTextSize(TEXT_SIZE);
		empiShadowTrailIntervalLabel->setTextSize(TEXT_SIZE);
		empiShadowTrailInterval->setTextSize(TEXT_SIZE);
		empiDamageLabel->setTextSize(TEXT_SIZE);
		empiDamage->setTextSize(TEXT_SIZE);
		empiOnCollisionActionLabel->setTextSize(TEXT_SIZE);
		empiOnCollisionAction->setTextSize(TEXT_SIZE);
		empiPierceResetTimeLabel->setTextSize(TEXT_SIZE);
		empiPierceResetTime->setTextSize(TEXT_SIZE);
		empiBulletModelLabel->setTextSize(TEXT_SIZE);
		empiBulletModel->setTextSize(TEXT_SIZE);
		empiInheritRadius->setTextSize(TEXT_SIZE);
		empiInheritDespawnTime->setTextSize(TEXT_SIZE);
		empiInheritShadowTrailInterval->setTextSize(TEXT_SIZE);
		empiInheritShadowTrailLifespan->setTextSize(TEXT_SIZE);
		empiInheritAnimatables->setTextSize(TEXT_SIZE);
		empiInheritDamage->setTextSize(TEXT_SIZE);
		empiInheritSoundSettings->setTextSize(TEXT_SIZE);

		id->setText("Movable point ID " + std::to_string(emp->getID()));
		empiHitboxRadiusLabel->setText("Hitbox radius");
		empiAnimatableLabel->setText("Sprite/Animation");
		empiBaseSpriteLabel->setText("Base sprite");
		empiActionsLabel->setText("Actions");
		empiActionsAdd->setText("Add");
		empiActionsDelete->setText("Delete");
		empiDespawnTimeLabel->setText("Time to despawn");
		empiSpawnTypeLabel->setText("Spawn type");
		empiSpawnTypeTimeLabel->setText("Spawn delay");
		empiSpawnTypeXLabel->setText("Spawn X");
		empiSpawnTypeYLabel->setText("Spawn Y");
		empiSpawnLocationManualSet->setText("Spawn position manual set");
		empiShadowTrailLifespanLabel->setText("Shadow trail lifespan");
		empiShadowTrailIntervalLabel->setText("Shadow trail spawn interval");
		empiDamageLabel->setText("Damage");
		empiOnCollisionActionLabel->setText("On-collision action");
		empiPierceResetTimeLabel->setText("Seconds between piercing hits");
		empiBulletModelLabel->setText("Bullet model");
		empiInheritRadius->setText("Inherit radius");
		empiInheritDespawnTime->setText("Inherit despawn time");
		empiInheritShadowTrailInterval->setText("Inherit shadow trail interval");
		empiInheritShadowTrailLifespan->setText("Inherit shadow trail lifespan");
		empiInheritAnimatables->setText("Inherit animatables");
		empiInheritDamage->setText("Inherit damage");
		empiInheritSoundSettings->setText("Inherit sound settings");

		empiSpawnType->addItem("Relative to map origin", "0");
		empiSpawnType->addItem("Detached, relative to parent", "1");
		empiSpawnType->addItem("Attached, relative to parent", "2");

		empiOnCollisionAction->addItem("Destroy self only", getID(DESTROY_THIS_BULLET_ONLY));
		empiOnCollisionAction->addItem("Destroy self and attached children", getID(DESTROY_THIS_BULLET_AND_ATTACHED_CHILDREN));
		empiOnCollisionAction->addItem("Pierce players/enemies", getID(PIERCE_ENTITY));

		empiBulletModel->addItem("None", "-1");
		for (auto it = levelPack.getBulletModelIteratorBegin(); it != levelPack.getBulletModelIteratorEnd(); it++) {
			empiBulletModel->addItem(it->second->getName(), std::to_string(it->second->getID()));
		}

		// Bullet model should be loaded whenever a change is made to the level pack
		levelPack.getOnChange()->sink().connect<EditorMovablePointPanel, &EditorMovablePointPanel::onLevelPackChange>(this);
		emp->loadBulletModel(levelPack);
		
		empiAnimatable->connect("ValueChanged", [this](Animatable value) {
			if (this->ignoreSignals) {
				return;
			}

			Animatable oldValue = this->emp->getAnimatable();
			undoStack.execute(UndoableCommand([this, value]() {
				this->emp->setAnimatable(value);

				this->ignoreSignals = true;
				empiAnimatable->setValue(value);
				empiLoopAnimation->setVisible(!value.isSprite());
				empiBaseSprite->setVisible(!empiLoopAnimation->isChecked() && !value.isSprite());
				empiBaseSpriteLabel->setVisible(empiBaseSprite->isVisible());
				this->ignoreSignals = false;

				onEMPModify.emit(this, this->emp);
			}, [this, oldValue]() {
				this->emp->setAnimatable(oldValue);

				this->ignoreSignals = true;
				empiAnimatable->setValue(oldValue);
				empiLoopAnimation->setVisible(!oldValue.isSprite());
				empiBaseSprite->setVisible(!empiLoopAnimation->isChecked() && !oldValue.isSprite());
				empiBaseSpriteLabel->setVisible(empiBaseSprite->isVisible());
				this->ignoreSignals = false;

				onEMPModify.emit(this, this->emp);
			}));
		});
		empiLoopAnimation->connect("Changed", [this](bool value) {
			if (this->ignoreSignals) {
				return;
			}

			bool oldValue = this->emp->getIsBullet();
			undoStack.execute(UndoableCommand([this, value]() {
				this->emp->setLoopAnimation(value);

				this->ignoreSignals = true;
				empiLoopAnimation->setChecked(value);
				empiBaseSprite->setVisible(!value && !empiAnimatable->getValue().isSprite());
				empiBaseSpriteLabel->setVisible(empiBaseSprite->isVisible());
				this->ignoreSignals = false;

				onEMPModify.emit(this, this->emp);
			}, [this, oldValue]() {
				this->emp->setLoopAnimation(oldValue);

				this->ignoreSignals = true;
				empiLoopAnimation->setChecked(oldValue);
				empiBaseSprite->setVisible(!oldValue && !empiAnimatable->getValue().isSprite());
				empiBaseSpriteLabel->setVisible(empiBaseSprite->isVisible());
				this->ignoreSignals = false;

				onEMPModify.emit(this, this->emp);
			}));
		});
		empiBaseSprite->connect("ValueChanged", [this](Animatable value) {
			if (this->ignoreSignals) {
				return;
			}

			Animatable oldValue = this->emp->getAnimatable();
			undoStack.execute(UndoableCommand([this, value]() {
				this->emp->setBaseSprite(value);
				this->ignoreSignals = true;
				empiBaseSprite->setValue(value);
				this->ignoreSignals = false;
				onEMPModify.emit(this, this->emp);
			}, [this, oldValue]() {
				this->emp->setBaseSprite(oldValue);
				this->ignoreSignals = true;
				empiBaseSprite->setValue(oldValue);
				this->ignoreSignals = false;
				onEMPModify.emit(this, this->emp);
			}));
		});
		isBullet->connect("Changed", [this](bool value) {
			if (this->ignoreSignals) {
				return;
			}

			bool oldValue = this->emp->getIsBullet();
			undoStack.execute(UndoableCommand([this, value]() {
				this->emp->setIsBullet(value);
				this->ignoreSignals = true;
				isBullet->setChecked(value);
				this->ignoreSignals = false;
				onEMPModify.emit(this, this->emp);
			}, [this, oldValue]() {
				this->emp->setIsBullet(oldValue);
				this->ignoreSignals = true;
				isBullet->setChecked(oldValue);
				this->ignoreSignals = false;
				onEMPModify.emit(this, this->emp);
			}));
		});
		empiHitboxRadius->connect("ValueChanged", [this](float value) {
			if (this->ignoreSignals) {
				return;
			}

			float oldValue = this->emp->getHitboxRadius();
			undoStack.execute(UndoableCommand([this, value]() {
				this->emp->setHitboxRadius(value);
				this->ignoreSignals = true;
				empiHitboxRadius->setValue(value);
				this->ignoreSignals = false;
				onEMPModify.emit(this, this->emp);
			}, [this, oldValue]() {
				this->emp->setHitboxRadius(oldValue);
				this->ignoreSignals = true;
				empiHitboxRadius->setValue(oldValue);
				this->ignoreSignals = false;
				onEMPModify.emit(this, this->emp);
			}));
		});
		empiActions->getListBox()->connect("ItemSelected", [&](int index) {
			selectedEMPAIndex = index;
		});
		empiActions->getListBox()->connect("DoubleClicked", [this](std::string item, std::string id) {
			std::shared_ptr<tgui::Panel> empaPanel = createEMPAPanel(this->emp->getActions()[std::stoi(id)]->clone(), std::stoi(id), empiActions);
			empaPanel->connect("EMPAModified", [this]() {
				populateEMPAList(empiActions);
			});
			tabs->addTab(format(EMPA_TAB_NAME_FORMAT, std::stoi(id)), empaPanel, true, true);
		});
		empiActionsAdd->connect("Pressed", [this]() {
			int newEMPAIndex;
			if (selectedEMPAIndex == -1) {
				newEMPAIndex = this->emp->getActionsCount();
			} else {
				newEMPAIndex = selectedEMPAIndex;
			}

			undoStack.execute(UndoableCommand([this, newEMPAIndex]() {
				this->emp->insertAction(newEMPAIndex, std::make_shared<StayStillAtLastPositionEMPA>(0));
				onEMPModify.emit(this, this->emp);

				// Rename all tabs for higher or equal EMPA indices
				std::vector<int> actionIndicesToBeRenamed;
				// Start at 1 because the first tab is the MP properties
				std::vector<std::string> tabNames = tabs->getTabNames();
				for (int i = 1; i < tabNames.size(); i++) {
					int actionIndex = std::stoi(tabNames[i].substr(EMPA_TAB_NAME_FORMAT_NUMBER_INDEX));
					if (actionIndex >= newEMPAIndex) {
						actionIndicesToBeRenamed.push_back(actionIndex);
					}
				}
				// Since action indicies need to be incremented, iterate starting at the highest action indices first so that
				// there can't be a situation where 2 tabs have the same name
				std::sort(actionIndicesToBeRenamed.begin(), actionIndicesToBeRenamed.end(), std::greater<int>());
				for (int actionIndex : actionIndicesToBeRenamed) {
					tabs->renameTab(format(EMPA_TAB_NAME_FORMAT, actionIndex), format(EMPA_TAB_NAME_FORMAT, actionIndex + 1));
				}

				this->populateEMPAList(empiActions);
			}, [this, newEMPAIndex]() {
				this->emp->removeAction(newEMPAIndex);
				onEMPModify.emit(this, this->emp);

				// Close the tab if it is open
				std::string tabName = format(EMPA_TAB_NAME_FORMAT, newEMPAIndex);
				if (tabs->hasTab(tabName)) {
					tabs->removeTab(tabName);
				}

				// Rename all tabs for higher EMPA indices
				std::vector<int> actionIndicesToBeRenamed;
				// Start at 1 because the first tab is the MP properties
				std::vector<std::string> tabNames = tabs->getTabNames();
				for (int i = 1; i < tabNames.size(); i++) {
					int actionIndex = std::stoi(tabNames[i].substr(EMPA_TAB_NAME_FORMAT_NUMBER_INDEX));
					if (actionIndex > newEMPAIndex) {
						actionIndicesToBeRenamed.push_back(actionIndex);
					}
				}
				// Since action indicies need to be decremented, iterate starting at the lowest action indices first so that
				// there can't be a situation where 2 tabs have the same name
				std::sort(actionIndicesToBeRenamed.begin(), actionIndicesToBeRenamed.end());
				for (int actionIndex : actionIndicesToBeRenamed) {
					tabs->renameTab(format(EMPA_TAB_NAME_FORMAT, actionIndex), format(EMPA_TAB_NAME_FORMAT, actionIndex - 1));
				}

				this->populateEMPAList(empiActions);
			}));
		});
		empiActionsDelete->connect("Pressed", [this, &index = this->selectedEMPAIndex]() {
			auto oldEMPA = this->emp->getAction(selectedEMPAIndex);
			undoStack.execute(UndoableCommand([this, index]() {
				this->emp->removeAction(index);
				onEMPModify.emit(this, this->emp);

				// Close the tab if it is open
				std::string tabName = format(EMPA_TAB_NAME_FORMAT, index);
				if (tabs->hasTab(tabName)) {
					tabs->removeTab(tabName);
				}

				// Rename all tabs for higher EMPA indices
				std::vector<int> actionIndicesToBeRenamed;
				// Start at 1 because the first tab is the MP properties
				std::vector<std::string> tabNames = tabs->getTabNames();
				for (int i = 1; i < tabNames.size(); i++) {
					int actionIndex = std::stoi(tabNames[i].substr(EMPA_TAB_NAME_FORMAT_NUMBER_INDEX));
					if (actionIndex > index) {
						actionIndicesToBeRenamed.push_back(actionIndex);
					}
				}
				// Since action indicies need to be decremented, iterate starting at the lowest action indices first so that
				// there can't be a situation where 2 tabs have the same name
				std::sort(actionIndicesToBeRenamed.begin(), actionIndicesToBeRenamed.end());
				for (int actionIndex : actionIndicesToBeRenamed) {
					tabs->renameTab(format(EMPA_TAB_NAME_FORMAT, actionIndex), format(EMPA_TAB_NAME_FORMAT, actionIndex - 1));
				}

				this->populateEMPAList(empiActions);
			}, [this, index, oldEMPA]() {
				this->emp->insertAction(index, oldEMPA);
				onEMPModify.emit(this, this->emp);

				// Rename all tabs for higher or equal EMPA indices
				std::vector<int> actionIndicesToBeRenamed;
				// Start at 1 because the first tab is the MP properties
				std::vector<std::string> tabNames = tabs->getTabNames();
				for (int i = 1; i < tabNames.size(); i++) {
					int actionIndex = std::stoi(tabNames[i].substr(EMPA_TAB_NAME_FORMAT_NUMBER_INDEX));
					if (actionIndex >= index) {
						actionIndicesToBeRenamed.push_back(actionIndex);
					}
				}
				// Since action indicies need to be incremented, iterate starting at the highest action indices first so that
				// there can't be a situation where 2 tabs have the same name
				std::sort(actionIndicesToBeRenamed.begin(), actionIndicesToBeRenamed.end(), std::greater<int>());
				for (int actionIndex : actionIndicesToBeRenamed) {
					tabs->renameTab(format(EMPA_TAB_NAME_FORMAT, actionIndex), format(EMPA_TAB_NAME_FORMAT, actionIndex + 1));
				}

				// Create the tab
				std::shared_ptr<tgui::Panel> empaPanel = createEMPAPanel(this->emp->getAction(index)->clone(), index, empiActions);
				tabs->addTab(format(EMPA_TAB_NAME_FORMAT, index), empaPanel, false, true);

				this->populateEMPAList(empiActions);
			}));
		});
		empiSpawnType->connect("ItemSelected", [this](std::string item, std::string id) {
			if (ignoreSignals) {
				return;
			}

			std::shared_ptr<EMPSpawnType> oldSpawnType = this->emp->getSpawnType();
			if (id == "0") {
				undoStack.execute(UndoableCommand(
					[this]() {
					this->emp->setSpawnType(std::make_shared<SpecificGlobalEMPSpawn>(empiSpawnTypeTime->getValue(), empiSpawnTypeX->getValue(), empiSpawnTypeY->getValue()));
					onEMPModify.emit(this, this->emp);

					ignoreSignals = true;
					empiSpawnType->setSelectedItemById(getID(this->emp->getSpawnType()));
					ignoreSignals = false;
				},
					[this, oldSpawnType]() {
					this->emp->setSpawnType(oldSpawnType);
					onEMPModify.emit(this, this->emp);

					ignoreSignals = true;
					empiSpawnType->setSelectedItemById(getID(this->emp->getSpawnType()));
					ignoreSignals = false;
				}));
			} else if (id == "1") {
				undoStack.execute(UndoableCommand(
					[this]() {
					this->emp->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>(empiSpawnTypeTime->getValue(), empiSpawnTypeX->getValue(), empiSpawnTypeY->getValue()));
					onEMPModify.emit(this, this->emp);

					ignoreSignals = true;
					empiSpawnType->setSelectedItemById(getID(this->emp->getSpawnType()));
					ignoreSignals = false;
				},
					[this, oldSpawnType]() {
					this->emp->setSpawnType(oldSpawnType);
					onEMPModify.emit(this, this->emp);

					ignoreSignals = true;
					empiSpawnType->setSelectedItemById(getID(this->emp->getSpawnType()));
					ignoreSignals = false;
				}));
			} else if (id == "2") {
				undoStack.execute(UndoableCommand(
					[this]() {
					this->emp->setSpawnType(std::make_shared<EntityAttachedEMPSpawn>(empiSpawnTypeTime->getValue(), empiSpawnTypeX->getValue(), empiSpawnTypeY->getValue()));
					onEMPModify.emit(this, this->emp);

					ignoreSignals = true;
					empiSpawnType->setSelectedItemById(getID(this->emp->getSpawnType()));
					ignoreSignals = false;
				},
					[this, oldSpawnType]() {
					this->emp->setSpawnType(oldSpawnType);
					onEMPModify.emit(this, this->emp);

					ignoreSignals = true;
					empiSpawnType->setSelectedItemById(getID(this->emp->getSpawnType()));
					ignoreSignals = false;
				}));
			} else {
				// You forgot a case
				assert(false);
			}
		});
		empiSpawnTypeTime->connect("ValueChanged", [this](float value) {
			if (ignoreSignals) {
				return;
			}

			float oldValue = this->emp->getSpawnType()->getTime();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setSpawnTypeTime(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSpawnTypeTime->setValue(this->emp->getSpawnType()->getTime());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->setSpawnTypeTime(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSpawnTypeTime->setValue(this->emp->getSpawnType()->getTime());
				ignoreSignals = false;
			}));
		});
		empiSpawnTypeX->connect("ValueChanged", [this](float value) {
			if (ignoreSignals) {
				return;
			}

			float oldValue = this->emp->getSpawnType()->getX();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->getSpawnType()->setX(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSpawnTypeX->setValue(this->emp->getSpawnType()->getX());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->getSpawnType()->setX(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSpawnTypeX->setValue(this->emp->getSpawnType()->getX());
				ignoreSignals = false;
			}));
		});
		empiSpawnTypeY->connect("ValueChanged", [this](float value) {
			if (ignoreSignals) {
				return;
			}

			float oldValue = this->emp->getSpawnType()->getY();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->getSpawnType()->setY(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSpawnTypeY->setValue(this->emp->getSpawnType()->getY());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->getSpawnType()->setY(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSpawnTypeY->setValue(this->emp->getSpawnType()->getY());
				ignoreSignals = false;
			}));
		});
		empiSpawnLocationManualSet->connect("Pressed", [&]() {
			savedWidgets = properties->getWidgets();
			horizontalScrollPos = properties->getHorizontalScrollbarValue();
			verticalScrollPos = properties->getVerticalScrollbarValue();

			properties->removeAllWidgets();
			properties->setHorizontalScrollbarValue(0);
			properties->setVerticalScrollbarValue(0);

			spawnTypePositionMarkerPlacer->clearUndoStack();
			spawnTypePositionMarkerPlacer->setMarkers({std::make_pair(sf::Vector2f(this->emp->getSpawnType()->getX(), this->emp->getSpawnType()->getY()), sf::Color::Red)});
			spawnTypePositionMarkerPlacer->lookAt(sf::Vector2f(0, 0));

			properties->add(spawnTypePositionMarkerPlacer);
			properties->add(spawnTypePositionMarkerPlacerFinishEditing);
			spawnTypePositionMarkerPlacer->setFocused(true);

			placingSpawnLocation = true;
		});
		empiShadowTrailLifespan->connect("ValueChanged", [this](float value) {
			if (ignoreSignals) {
				return;
			}

			float oldValue = this->emp->getShadowTrailLifespan();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setShadowTrailLifespan(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiShadowTrailLifespan->setValue(this->emp->getShadowTrailLifespan());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->setShadowTrailLifespan(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiShadowTrailLifespan->setValue(this->emp->getShadowTrailLifespan());
				ignoreSignals = false;
			}));
		});
		empiShadowTrailInterval->connect("ValueChanged", [this](float value) {
			if (ignoreSignals) {
				return;
			}

			float oldValue = this->emp->getShadowTrailInterval();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setShadowTrailInterval(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiShadowTrailInterval->setValue(this->emp->getShadowTrailInterval());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->setShadowTrailInterval(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiShadowTrailInterval->setValue(this->emp->getShadowTrailInterval());
				ignoreSignals = false;
			}));
		});
		empiDamage->connect("ValueChanged", [this](float value) {
			if (ignoreSignals) {
				return;
			}

			float oldValue = this->emp->getDamage();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setDamage(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiDamage->setValue(this->emp->getDamage());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->setDamage(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiDamage->setValue(this->emp->getDamage());
				ignoreSignals = false;
			}));
		});
		empiOnCollisionAction->connect("ItemSelected", [this](std::string item, std::string id) {
			if (ignoreSignals) {
				return;
			}

			BULLET_ON_COLLISION_ACTION action = fromID(empiOnCollisionAction->getSelectedItemId());
			BULLET_ON_COLLISION_ACTION oldAction = this->emp->getOnCollisionAction();
			undoStack.execute(UndoableCommand(
				[this, action]() {
				this->emp->setOnCollisionAction(action);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiOnCollisionAction->setSelectedItemById(getID(action));
				empiPierceResetTimeLabel->setVisible(action == PIERCE_ENTITY);
				empiPierceResetTime->setVisible(action == PIERCE_ENTITY);
				ignoreSignals = false;
			},
				[this, oldAction]() {
				this->emp->setOnCollisionAction(oldAction);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiOnCollisionAction->setSelectedItemById(getID(oldAction));
				empiPierceResetTimeLabel->setVisible(oldAction == PIERCE_ENTITY);
				empiPierceResetTime->setVisible(oldAction == PIERCE_ENTITY);
				ignoreSignals = false;
			}));
		});
		empiPierceResetTime->connect("ValueChanged", [this](float value) {
			if (ignoreSignals) {
				return;
			}

			float oldValue = this->emp->getPierceResetTime();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setPierceResetTime(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiPierceResetTime->setValue(this->emp->getPierceResetTime());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->setPierceResetTime(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiPierceResetTime->setValue(this->emp->getPierceResetTime());
				ignoreSignals = false;
			}));
		});
		empiSoundSettings->connect("ValueChanged", [this](SoundSettings value) {
			if (ignoreSignals) {
				return;
			}

			SoundSettings oldValue = this->emp->getSoundSettings();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setSoundSettings(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSoundSettings->initSettings(this->emp->getSoundSettings());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->setSoundSettings(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSoundSettings->initSettings(this->emp->getSoundSettings());
				ignoreSignals = false;
			}));
		});
		empiBulletModel->connect("ItemSelected", [this](std::string item, std::string id) {
			if (ignoreSignals) {
				return;
			}

			int bulletModelID = std::stoi(id);
			if (item == "") bulletModelID = -1;
			int oldBulletModelID = this->emp->getBulletModelID();
			float radius = this->emp->getHitboxRadius();
			float despawnTime = this->emp->getDespawnTime();
			float interval = this->emp->getShadowTrailInterval();
			float lifespan = this->emp->getShadowTrailLifespan();
			Animatable animatable = this->emp->getAnimatable();
			Animatable baseSprite = this->emp->getBaseSprite();
			bool loopAnimation = this->emp->getLoopAnimation();
			float damage = this->emp->getDamage();
			SoundSettings sound = this->emp->getSoundSettings();
			undoStack.execute(UndoableCommand(
				[this, bulletModelID, radius, despawnTime, interval, lifespan, animatable, baseSprite, loopAnimation, damage, sound]() {
				if (bulletModelID == -1) {
					this->emp->removeBulletModel();
				} else {
					this->emp->setBulletModel(this->levelPack.getBulletModel(bulletModelID));
				}
				onEMPModify.emit(this, this->emp);


				ignoreSignals = true;
				if (bulletModelID == -1) {
					empiBulletModel->deselectItem();
				} else {
					empiBulletModel->setSelectedItemById(std::to_string(this->emp->getBulletModelID()));
				}
				empiHitboxRadius->setValue(this->emp->getHitboxRadius());
				empiDespawnTime->setValue(this->emp->getDespawnTime());
				empiShadowTrailLifespan->setValue(this->emp->getShadowTrailLifespan());
				empiShadowTrailInterval->setValue(this->emp->getShadowTrailInterval());
				empiAnimatable->setValue(this->emp->getAnimatable());
				empiLoopAnimation->setChecked(this->emp->getLoopAnimation());
				empiBaseSprite->setValue(this->emp->getAnimatable());
				empiDamage->setValue(this->emp->getDamage());
				empiSoundSettings->initSettings(this->emp->getSoundSettings());

				empiLoopAnimation->setVisible(!this->emp->getAnimatable().isSprite());
				empiBaseSprite->setVisible(!this->emp->getLoopAnimation() && !this->emp->getAnimatable().isSprite());
				empiBaseSpriteLabel->setVisible(empiBaseSprite->isVisible());
				empiPierceResetTimeLabel->setVisible(this->emp->getOnCollisionAction() == PIERCE_ENTITY);
				empiPierceResetTime->setVisible(this->emp->getOnCollisionAction() == PIERCE_ENTITY);

				empiHitboxRadius->setEnabled(!this->emp->getInheritRadius() || this->emp->getBulletModelID() < 0);
				empiDespawnTime->setEnabled(!this->emp->getInheritDespawnTime() || this->emp->getBulletModelID() < 0);
				empiShadowTrailInterval->setEnabled(!this->emp->getInheritShadowTrailInterval() || this->emp->getBulletModelID() < 0);
				empiShadowTrailLifespan->setEnabled(!this->emp->getInheritShadowTrailLifespan() || this->emp->getBulletModelID() < 0);
				empiAnimatable->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
				empiLoopAnimation->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
				empiBaseSprite->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
				empiDamage->setEnabled(!this->emp->getInheritDamage() || this->emp->getBulletModelID() < 0);
				empiSoundSettings->setEnabled(!this->emp->getInheritSoundSettings() || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			},
				[this, oldBulletModelID, radius, despawnTime, interval, lifespan, animatable, baseSprite, loopAnimation, damage, sound]() {
				this->emp->setHitboxRadius(radius);
				this->emp->setDespawnTime(despawnTime);
				this->emp->setShadowTrailLifespan(lifespan);
				this->emp->setShadowTrailInterval(interval);
				this->emp->setAnimatable(animatable);
				this->emp->setLoopAnimation(loopAnimation);
				this->emp->setBaseSprite(baseSprite);
				this->emp->setDamage(damage);
				this->emp->setSoundSettings(sound);
				if (oldBulletModelID == -1) {
					this->emp->removeBulletModel();
				} else {
					this->emp->setBulletModel(this->levelPack.getBulletModel(oldBulletModelID));
				}
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				if (oldBulletModelID == -1) {
					empiBulletModel->deselectItem();
				} else {
					empiBulletModel->setSelectedItemById(std::to_string(this->emp->getBulletModelID()));
				}
				empiHitboxRadius->setValue(this->emp->getHitboxRadius());
				empiDespawnTime->setValue(this->emp->getDespawnTime());
				empiShadowTrailLifespan->setValue(this->emp->getShadowTrailLifespan());
				empiShadowTrailInterval->setValue(this->emp->getShadowTrailInterval());
				empiAnimatable->setValue(this->emp->getAnimatable());
				empiLoopAnimation->setChecked(this->emp->getLoopAnimation());
				empiBaseSprite->setValue(this->emp->getAnimatable());
				empiDamage->setValue(this->emp->getDamage());
				empiSoundSettings->initSettings(this->emp->getSoundSettings());

				empiLoopAnimation->setVisible(!this->emp->getAnimatable().isSprite());
				empiBaseSprite->setVisible(!this->emp->getLoopAnimation() && !this->emp->getAnimatable().isSprite());
				empiBaseSpriteLabel->setVisible(empiBaseSprite->isVisible());
				empiPierceResetTimeLabel->setVisible(this->emp->getOnCollisionAction() == PIERCE_ENTITY);
				empiPierceResetTime->setVisible(this->emp->getOnCollisionAction() == PIERCE_ENTITY);

				empiHitboxRadius->setEnabled(!this->emp->getInheritRadius() || this->emp->getBulletModelID() < 0);
				empiDespawnTime->setEnabled(!this->emp->getInheritDespawnTime() || this->emp->getBulletModelID() < 0);
				empiShadowTrailInterval->setEnabled(!this->emp->getInheritShadowTrailInterval() || this->emp->getBulletModelID() < 0);
				empiShadowTrailLifespan->setEnabled(!this->emp->getInheritShadowTrailLifespan() || this->emp->getBulletModelID() < 0);
				empiAnimatable->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
				empiLoopAnimation->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
				empiBaseSprite->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
				empiDamage->setEnabled(!this->emp->getInheritDamage() || this->emp->getBulletModelID() < 0);
				empiSoundSettings->setEnabled(!this->emp->getInheritSoundSettings() || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			}));
		});
		empiInheritRadius->connect("Changed", [this](bool value) {
			if (ignoreSignals) {
				return;
			}

			bool oldValue = this->emp->getInheritRadius();
			float oldInheritValue = this->emp->getHitboxRadius();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setInheritRadius(value, this->levelPack);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritRadius->setChecked(this->emp->getInheritRadius());
				empiHitboxRadius->setValue(this->emp->getHitboxRadius());
				empiHitboxRadius->setEnabled(!value || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			},
				[this, oldValue, oldInheritValue]() {
				this->emp->setInheritRadius(oldValue, this->levelPack);
				if (!oldValue) {
					this->emp->setHitboxRadius(oldInheritValue);
				}
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritRadius->setChecked(this->emp->getInheritRadius());
				empiHitboxRadius->setValue(this->emp->getHitboxRadius());
				empiHitboxRadius->setEnabled(!oldValue || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			}));
		});
		empiInheritDespawnTime->connect("Changed", [this](bool value) {
			if (ignoreSignals) {
				return;
			}

			bool oldValue = this->emp->getInheritDespawnTime();
			float oldInheritValue = this->emp->getDespawnTime();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setInheritDespawnTime(value, this->levelPack);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritDespawnTime->setChecked(this->emp->getInheritDespawnTime());
				empiDespawnTime->setValue(this->emp->getDespawnTime());
				empiDespawnTime->setEnabled(!value || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			},
				[this, oldValue, oldInheritValue]() {
				this->emp->setInheritDespawnTime(oldValue, this->levelPack);
				if (!oldValue) {
					this->emp->setDespawnTime(oldInheritValue);
				}
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritDespawnTime->setChecked(this->emp->getInheritDespawnTime());
				empiDespawnTime->setValue(this->emp->getDespawnTime());
				empiDespawnTime->setEnabled(!oldValue || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			}));
		});
		empiInheritShadowTrailInterval->connect("Changed", [this](bool value) {
			if (ignoreSignals) {
				return;
			}

			bool oldValue = this->emp->getInheritShadowTrailInterval();
			float oldInheritValue = this->emp->getShadowTrailInterval();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setInheritShadowTrailInterval(value, this->levelPack);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritShadowTrailInterval->setChecked(this->emp->getInheritShadowTrailInterval());
				empiShadowTrailInterval->setValue(this->emp->getShadowTrailInterval());
				empiShadowTrailInterval->setEnabled(!value || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			},
				[this, oldValue, oldInheritValue]() {
				this->emp->setInheritShadowTrailInterval(oldValue, this->levelPack);
				if (!oldValue) {
					this->emp->setShadowTrailInterval(oldInheritValue);
				}
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritShadowTrailInterval->setChecked(this->emp->getInheritShadowTrailInterval());
				empiShadowTrailInterval->setValue(this->emp->getShadowTrailInterval());
				empiShadowTrailInterval->setEnabled(!oldValue || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			}));
		});
		empiInheritShadowTrailLifespan->connect("Changed", [this](bool value) {
			if (ignoreSignals) {
				return;
			}

			bool oldValue = this->emp->getInheritShadowTrailLifespan();
			float oldInheritValue = this->emp->getShadowTrailLifespan();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setInheritShadowTrailLifespan(value, this->levelPack);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritShadowTrailLifespan->setChecked(this->emp->getInheritShadowTrailLifespan());
				empiShadowTrailLifespan->setValue(this->emp->getShadowTrailLifespan());
				empiShadowTrailLifespan->setEnabled(!value || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			},
				[this, oldValue, oldInheritValue]() {
				this->emp->setInheritShadowTrailLifespan(oldValue, this->levelPack);
				if (!oldValue) {
					this->emp->setShadowTrailLifespan(oldInheritValue);
				}
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritShadowTrailLifespan->setChecked(this->emp->getInheritShadowTrailLifespan());
				empiShadowTrailLifespan->setValue(this->emp->getShadowTrailLifespan());
				empiShadowTrailLifespan->setEnabled(!oldValue || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			}));
		});
		empiInheritAnimatables->connect("Changed", [this](bool value) {
			if (ignoreSignals) {
				return;
			}

			bool oldValue = this->emp->getInheritAnimatables();
			Animatable oldAnimatable = this->emp->getAnimatable();
			Animatable oldBaseSprite = this->emp->getBaseSprite();
			bool oldLoopAnimation = this->emp->getLoopAnimation();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setInheritAnimatables(value, this->levelPack);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritAnimatables->setChecked(this->emp->getInheritAnimatables());
				empiAnimatable->setValue(this->emp->getAnimatable());
				empiBaseSprite->setValue(this->emp->getBaseSprite());
				empiLoopAnimation->setChecked(this->emp->getLoopAnimation());

				empiLoopAnimation->setVisible(!this->emp->getAnimatable().isSprite());
				empiBaseSprite->setVisible(!this->emp->getLoopAnimation() && !this->emp->getAnimatable().isSprite());
				empiBaseSpriteLabel->setVisible(empiBaseSprite->isVisible());

				empiAnimatable->setEnabled(!value || this->emp->getBulletModelID() < 0);
				empiLoopAnimation->setEnabled(!value || this->emp->getBulletModelID() < 0);
				empiBaseSprite->setEnabled(!value || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			},
				[this, oldValue, oldAnimatable, oldBaseSprite, oldLoopAnimation]() {
				this->emp->setInheritAnimatables(oldValue, this->levelPack);
				if (!oldValue) {
					this->emp->setAnimatable(oldAnimatable);
					this->emp->setBaseSprite(oldBaseSprite);
					this->emp->setLoopAnimation(oldLoopAnimation);
				}
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritAnimatables->setChecked(this->emp->getInheritAnimatables());
				empiAnimatable->setValue(this->emp->getAnimatable());
				empiBaseSprite->setValue(this->emp->getBaseSprite());
				empiLoopAnimation->setChecked(this->emp->getLoopAnimation());

				empiLoopAnimation->setVisible(!this->emp->getAnimatable().isSprite());
				empiBaseSprite->setVisible(!this->emp->getLoopAnimation() && !this->emp->getAnimatable().isSprite());
				empiBaseSpriteLabel->setVisible(empiBaseSprite->isVisible());

				empiAnimatable->setEnabled(!oldValue || this->emp->getBulletModelID() < 0);
				empiLoopAnimation->setEnabled(!oldValue || this->emp->getBulletModelID() < 0);
				empiBaseSprite->setEnabled(!oldValue || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			}));
		});
		empiInheritDamage->connect("Changed", [this](bool value) {
			if (ignoreSignals) {
				return;
			}

			bool oldValue = this->emp->getInheritDamage();
			float oldInheritValue = this->emp->getDamage();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setInheritDamage(value, this->levelPack);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritDamage->setChecked(this->emp->getInheritDamage());
				empiDamage->setValue(this->emp->getDamage());
				empiDamage->setEnabled(!value || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			},
				[this, oldValue, oldInheritValue]() {
				this->emp->setInheritDamage(oldValue, this->levelPack);
				if (!oldValue) {
					this->emp->setDamage(oldInheritValue);
				}
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritDamage->setChecked(this->emp->getInheritDamage());
				empiDamage->setValue(this->emp->getDamage());
				empiDamage->setEnabled(!oldValue || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			}));
		});
		empiInheritSoundSettings->connect("Changed", [this](bool value) {
			if (ignoreSignals) {
				return;
			}

			bool oldValue = this->emp->getInheritSoundSettings();
			SoundSettings oldInheritValue = this->emp->getSoundSettings();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setInheritSoundSettings(value, this->levelPack);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritSoundSettings->setChecked(this->emp->getInheritSoundSettings());
				empiSoundSettings->initSettings(this->emp->getSoundSettings());
				empiSoundSettings->setEnabled(!value || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			},
				[this, oldValue, oldInheritValue]() {
				this->emp->setInheritSoundSettings(oldValue, this->levelPack);
				if (!oldValue) {
					this->emp->setSoundSettings(oldInheritValue);
				}
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritSoundSettings->setChecked(this->emp->getInheritSoundSettings());
				empiSoundSettings->initSettings(this->emp->getSoundSettings());
				empiSoundSettings->setEnabled(!oldValue || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			}));
		});

		updateAllWidgetValues();

		id->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
		empiAnimatableLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(id) + GUI_PADDING_Y);
		empiAnimatable->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiAnimatableLabel) + GUI_LABEL_PADDING_Y);
		empiLoopAnimation->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiAnimatable) + GUI_PADDING_Y);
		empiBaseSpriteLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiLoopAnimation) + GUI_PADDING_Y * 2);
		empiBaseSprite->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiBaseSpriteLabel) + GUI_LABEL_PADDING_Y);
		isBullet->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiBaseSprite) + GUI_PADDING_Y * 2);

		empiBulletModelLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(isBullet) + GUI_PADDING_Y);
		empiBulletModel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiBulletModelLabel) + GUI_LABEL_PADDING_Y);
		empiInheritRadius->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiBulletModel) + GUI_LABEL_PADDING_Y);
		empiInheritDespawnTime->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritRadius) + GUI_LABEL_PADDING_Y);
		empiInheritShadowTrailInterval->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritDespawnTime) + GUI_LABEL_PADDING_Y);
		empiInheritShadowTrailLifespan->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritShadowTrailInterval) + GUI_LABEL_PADDING_Y);
		empiInheritAnimatables->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritShadowTrailLifespan) + GUI_LABEL_PADDING_Y);
		empiInheritDamage->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritAnimatables) + GUI_LABEL_PADDING_Y);
		empiInheritSoundSettings->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritDamage) + GUI_LABEL_PADDING_Y);

		empiHitboxRadiusLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritSoundSettings) + GUI_PADDING_Y);
		empiHitboxRadius->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiHitboxRadiusLabel) + GUI_LABEL_PADDING_Y);
		empiActionsLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiHitboxRadius) + GUI_PADDING_Y);
		empiActions->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiActionsLabel) + GUI_LABEL_PADDING_Y);
		empiActionsAdd->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiActions) + GUI_PADDING_Y);
		empiActionsDelete->setPosition(tgui::bindRight(empiActionsAdd) + GUI_PADDING_X, tgui::bindTop(empiActionsAdd));
		empiDespawnTimeLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiActionsDelete) + GUI_PADDING_Y);
		empiDespawnTime->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiDespawnTimeLabel) + GUI_LABEL_PADDING_Y);
		empiSpawnTypeLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiDespawnTime) + GUI_PADDING_Y * 2);
		empiSpawnType->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiSpawnTypeLabel) + GUI_LABEL_PADDING_Y);
		empiSpawnTypeTimeLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiSpawnType) + GUI_PADDING_Y);
		empiSpawnTypeTime->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiSpawnTypeTimeLabel) + GUI_LABEL_PADDING_Y);
		empiSpawnTypeXLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiSpawnTypeTime) + GUI_PADDING_Y);
		empiSpawnTypeX->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiSpawnTypeXLabel) + GUI_LABEL_PADDING_Y);
		empiSpawnTypeYLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiSpawnTypeX) + GUI_PADDING_Y);
		empiSpawnTypeY->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiSpawnTypeYLabel) + GUI_LABEL_PADDING_Y);
		empiSpawnLocationManualSet->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiSpawnTypeY) + GUI_PADDING_Y);
		empiShadowTrailLifespanLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiSpawnLocationManualSet) + GUI_PADDING_Y * 2);
		empiShadowTrailLifespan->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiShadowTrailLifespanLabel) + GUI_LABEL_PADDING_Y);
		empiShadowTrailIntervalLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiShadowTrailLifespan) + GUI_PADDING_Y);
		empiShadowTrailInterval->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiShadowTrailIntervalLabel) + GUI_LABEL_PADDING_Y);
		empiDamageLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiShadowTrailInterval) + GUI_PADDING_Y * 2);
		empiDamage->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiDamageLabel) + GUI_LABEL_PADDING_Y);
		empiOnCollisionActionLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiDamage) + GUI_PADDING_Y);
		empiOnCollisionAction->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiOnCollisionActionLabel) + GUI_LABEL_PADDING_Y);
		empiPierceResetTimeLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiOnCollisionAction) + GUI_PADDING_Y);
		empiPierceResetTime->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiPierceResetTimeLabel) + GUI_LABEL_PADDING_Y);
		empiSoundSettings->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiPierceResetTime) + GUI_PADDING_Y);

		tgui::Layout fillWidth = tgui::bindWidth(properties) - GUI_PADDING_X * 2;
		empiLoopAnimation->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiAnimatable->setSize(fillWidth, 0);
		empiBaseSprite->setSize(fillWidth, 0);
		empiAnimatable->setAnimatablePictureSize(fillWidth, tgui::bindMin(tgui::bindWidth(properties) - GUI_PADDING_X * 2, 120));
		empiBaseSprite->setAnimatablePictureSize(fillWidth, tgui::bindMin(tgui::bindWidth(properties) - GUI_PADDING_X * 2, 120));
		empiHitboxRadius->setSize(fillWidth, TEXT_BOX_HEIGHT);
		empiActions->setSize(fillWidth, 250);
		empiActionsAdd->setSize((tgui::bindWidth(empiActions) - GUI_PADDING_X)/2.0f, TEXT_BUTTON_HEIGHT);
		empiActionsDelete->setSize((tgui::bindWidth(empiActions) - GUI_PADDING_X) / 2.0f, TEXT_BUTTON_HEIGHT);
		empiDespawnTime->setSize(fillWidth, TEXT_BOX_HEIGHT);
		empiSpawnType->setSize(fillWidth, TEXT_BOX_HEIGHT);
		empiSpawnTypeTime->setSize(fillWidth, TEXT_BOX_HEIGHT);
		empiSpawnTypeX->setSize(fillWidth, TEXT_BOX_HEIGHT);
		empiSpawnTypeY->setSize(fillWidth, TEXT_BOX_HEIGHT);
		empiSpawnLocationManualSet->setSize(fillWidth, TEXT_BUTTON_HEIGHT);
		empiShadowTrailLifespan->setSize(fillWidth, TEXT_BOX_HEIGHT);
		empiShadowTrailInterval->setSize(fillWidth, TEXT_BOX_HEIGHT);
		empiDamage->setSize(fillWidth, TEXT_BOX_HEIGHT);
		empiOnCollisionAction->setSize(fillWidth, TEXT_BOX_HEIGHT);
		empiPierceResetTime->setSize(fillWidth, TEXT_BOX_HEIGHT);
		empiSoundSettings->setSize(fillWidth, 0);
		empiBulletModel->setSize(fillWidth, TEXT_BOX_HEIGHT);
		empiInheritRadius->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiInheritDespawnTime->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiInheritShadowTrailInterval->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiInheritShadowTrailLifespan->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiInheritAnimatables->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiInheritDamage->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiInheritSoundSettings->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);

		properties->add(id);
		properties->add(empiAnimatableLabel);
		properties->add(empiAnimatable);
		properties->add(empiLoopAnimation);
		properties->add(empiBaseSpriteLabel);
		properties->add(empiBaseSprite);
		properties->add(isBullet);
		properties->add(empiHitboxRadiusLabel);
		properties->add(empiHitboxRadius);
		properties->add(empiActionsLabel);
		properties->add(empiActions);
		properties->add(empiActionsAdd);
		properties->add(empiActionsDelete);
		properties->add(empiDespawnTimeLabel);
		properties->add(empiDespawnTime);
		properties->add(empiSpawnTypeLabel);
		properties->add(empiSpawnType);
		properties->add(empiSpawnTypeTimeLabel);
		properties->add(empiSpawnTypeTime);
		properties->add(empiSpawnTypeXLabel);
		properties->add(empiSpawnTypeX);
		properties->add(empiSpawnTypeYLabel);
		properties->add(empiSpawnTypeY);
		properties->add(empiSpawnLocationManualSet);
		properties->add(empiShadowTrailLifespanLabel);
		properties->add(empiShadowTrailLifespan);
		properties->add(empiShadowTrailIntervalLabel);
		properties->add(empiShadowTrailInterval);
		properties->add(empiDamageLabel);
		properties->add(empiDamage);
		properties->add(empiOnCollisionActionLabel);
		properties->add(empiOnCollisionAction);
		properties->add(empiPierceResetTimeLabel);
		properties->add(empiPierceResetTime);
		properties->add(empiSoundSettings);
		properties->add(empiBulletModelLabel);
		properties->add(empiBulletModel);
		properties->add(empiInheritRadius);
		properties->add(empiInheritDespawnTime);
		properties->add(empiInheritShadowTrailInterval);
		properties->add(empiInheritShadowTrailLifespan);
		properties->add(empiInheritAnimatables);
		properties->add(empiInheritDamage);
		properties->add(empiInheritSoundSettings);

		properties->connect("SizeChanged", [&](sf::Vector2f newSize) {
			spawnTypePositionMarkerPlacerFinishEditing->setPosition(newSize.x - spawnTypePositionMarkerPlacerFinishEditing->getSize().x, newSize.y - spawnTypePositionMarkerPlacerFinishEditing->getSize().y);
		});

		tabs->addTab(PROPERTIES_TAB_NAME, properties);
	}
}

EditorMovablePointPanel::~EditorMovablePointPanel() {
	levelPack.getOnChange()->sink().disconnect<EditorMovablePointPanel, &EditorMovablePointPanel::onLevelPackChange>(this);
}

std::shared_ptr<CopiedObject> EditorMovablePointPanel::copyFrom() {
	// Can't copy this widget
	return nullptr;
}

void EditorMovablePointPanel::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	// Same functionality as paste2Into()
	paste2Into(pastedObject);
}

void EditorMovablePointPanel::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	// Paste the first copied EditorMovablePoint to override emp's properties
	auto derived = std::static_pointer_cast<CopiedEditorMovablePoint>(pastedObject);
	if (derived) {
		std::shared_ptr<EditorMovablePoint> copiedEMP = derived->getEMP();

		mainEditorWindow.promptConfirmation("Overwrite this movable point's properties with the copied movable point's properties? This will not change this movable point's children.", copiedEMP)->sink()
			.connect<EditorMovablePointPanel, &EditorMovablePointPanel::onPasteIntoConfirmation>(this);
	}
}

bool EditorMovablePointPanel::handleEvent(sf::Event event) {
	if (placingSpawnLocation) {
		if (spawnTypePositionMarkerPlacer->handleEvent(event)) {
			return true;
		}

		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
			finishEditingSpawnTypePosition();
		}
	} else if (tabs->handleEvent(event)) {
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
				clipboard.paste(this);
				return true;
			}
		}
	}
	return false;
}

tgui::Signal & EditorMovablePointPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onEMPModify.getName())) {
		return onEMPModify;
	}
	return tgui::Panel::getSignal(signalName);
}

void EditorMovablePointPanel::populateEMPAList(std::shared_ptr<ListBoxScrollablePanel> actionsListBoxScrollablePanel) {
	bool oldIgnoreSignals = ignoreSignals;
	ignoreSignals = true;
	actionsListBoxScrollablePanel->getListBox()->removeAllItems();
	auto actions = emp->getActions();
	float curTime = 0;
	for (int i = 0; i < actions.size(); i++) {
		actionsListBoxScrollablePanel->getListBox()->addItem(actions[i]->getGuiFormat() + " (d=" + formatNum(actions[i]->getTime()) + "; t=" + formatNum(curTime) + " to t=" + formatNum(curTime + actions[i]->getTime()) + ")", std::to_string(i));
		curTime += actions[i]->getTime();
	}
	actionsListBoxScrollablePanel->onListBoxItemsUpdate();
	// Reselect previous EMPA
	if (selectedEMPAIndex != -1) {
		actionsListBoxScrollablePanel->getListBox()->setSelectedItemByIndex(selectedEMPAIndex);
	}
	// Set max time for despawn time
	empiDespawnTime->setMax(curTime);
	ignoreSignals = oldIgnoreSignals;
}

void EditorMovablePointPanel::updateAllWidgetValues() {
	// Update widgets whose values can be changed by the player
	ignoreSignals = true;
	empiAnimatable->setValue(emp->getAnimatable());
	empiLoopAnimation->setChecked(emp->getLoopAnimation());
	empiBaseSprite->setValue(emp->getAnimatable());
	isBullet->setChecked(emp->getIsBullet());
	empiHitboxRadius->setValue(emp->getHitboxRadius());
	populateEMPAList(empiActions);
	empiDespawnTime->setValue(emp->getDespawnTime());
	empiSpawnType->setSelectedItemById(getID(emp->getSpawnType()));
	empiSpawnTypeTime->setValue(emp->getSpawnType()->getTime());
	empiSpawnTypeX->setValue(emp->getSpawnType()->getX());
	empiSpawnTypeY->setValue(emp->getSpawnType()->getY());
	empiShadowTrailLifespan->setValue(emp->getShadowTrailLifespan());
	empiShadowTrailInterval->setValue(emp->getShadowTrailInterval());
	empiDamage->setValue(emp->getDamage());
	empiOnCollisionAction->setSelectedItemById(getID(emp->getOnCollisionAction()));
	empiPierceResetTime->setValue(emp->getPierceResetTime());
	if (emp->getBulletModelID() >= 0) {
		empiBulletModel->setSelectedItemById(std::to_string(emp->getBulletModelID()));
	} else {
		empiBulletModel->setSelectedItemById("");
	}
	empiInheritRadius->setChecked(emp->getInheritRadius());
	empiInheritDespawnTime->setChecked(emp->getInheritDespawnTime());
	empiInheritShadowTrailInterval->setChecked(emp->getInheritShadowTrailInterval());
	empiInheritShadowTrailLifespan->setChecked(emp->getInheritShadowTrailLifespan());
	empiInheritAnimatables->setChecked(emp->getInheritAnimatables());
	empiInheritDamage->setChecked(emp->getInheritDamage());
	empiInheritSoundSettings->setChecked(emp->getInheritSoundSettings());

	empiHitboxRadius->setEnabled(!emp->getInheritRadius() || this->emp->getBulletModelID() < 0);
	empiDespawnTime->setEnabled(!emp->getInheritDespawnTime() || this->emp->getBulletModelID() < 0);
	empiShadowTrailInterval->setEnabled(!emp->getInheritShadowTrailInterval() || this->emp->getBulletModelID() < 0);
	empiShadowTrailLifespan->setEnabled(!emp->getInheritShadowTrailLifespan() || this->emp->getBulletModelID() < 0);
	empiAnimatable->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
	empiLoopAnimation->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
	empiBaseSprite->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
	empiDamage->setEnabled(!emp->getInheritDamage() || this->emp->getBulletModelID() < 0);
	empiSoundSettings->setEnabled(!emp->getInheritSoundSettings() || this->emp->getBulletModelID() < 0);

	empiLoopAnimation->setVisible(!emp->getAnimatable().isSprite());
	empiBaseSprite->setVisible(!emp->getLoopAnimation() && !emp->getAnimatable().isSprite());
	empiBaseSpriteLabel->setVisible(empiBaseSprite->isVisible());
	empiPierceResetTimeLabel->setVisible(emp->getOnCollisionAction() == PIERCE_ENTITY);
	empiPierceResetTime->setVisible(emp->getOnCollisionAction() == PIERCE_ENTITY);
	ignoreSignals = false;
}

std::shared_ptr<tgui::Panel> EditorMovablePointPanel::createEMPAPanel(std::shared_ptr<EMPAction> empa, int index, std::shared_ptr<ListBoxScrollablePanel> empiActions) {
	std::shared_ptr<EditorMovablePointActionPanel> empaPanel = EditorMovablePointActionPanel::create(this->mainEditorWindow, empa);
	empaPanel->connect("EMPAModified", [this, index, empiActions](std::shared_ptr<EMPAction> value) {
		if (this->ignoreSignals) {
			return;
		}

		this->emp->replaceAction(index, value);
		this->populateEMPAList(empiActions);
		onEMPModify.emit(this, this->emp);
	});
	return empaPanel;
}

void EditorMovablePointPanel::onLevelPackChange() {
	emp->loadBulletModel(levelPack);

	empiSoundSettings->populateFileNames(format(LEVEL_PACK_SOUND_FOLDER_PATH, levelPack.getName().c_str()));

	empiBulletModel->removeAllItems();
	empiBulletModel->addItem("None", "-1");
	for (auto it = levelPack.getBulletModelIteratorBegin(); it != levelPack.getBulletModelIteratorEnd(); it++) {
		empiBulletModel->addItem(it->second->getName(), std::to_string(it->second->getID()));
	}
}

void EditorMovablePointPanel::finishEditingSpawnTypePosition() {
	properties->removeAllWidgets();
	for (auto widget : savedWidgets) {
		properties->add(widget);
	}
	savedWidgets.clear();
	properties->setHorizontalScrollbarValue(horizontalScrollPos);
	properties->setVerticalScrollbarValue(verticalScrollPos);

	float oldPosX = emp->getSpawnType()->getX();
	float oldPosY = emp->getSpawnType()->getY();
	sf::Vector2f newPos = spawnTypePositionMarkerPlacer->getMarkerPositions()[0];
	undoStack.execute(UndoableCommand(
		[this, newPos]() {
		emp->getSpawnType()->setX(newPos.x);
		emp->getSpawnType()->setY(newPos.y);
		onEMPModify.emit(this, this->emp);

		this->ignoreSignals = true;
		empiSpawnTypeX->setValue(newPos.x);
		empiSpawnTypeY->setValue(newPos.y);
		this->ignoreSignals = false;
	},
		[this, oldPosX, oldPosY]() {
		emp->getSpawnType()->setX(oldPosX);
		emp->getSpawnType()->setY(oldPosY);
		onEMPModify.emit(this, this->emp);

		this->ignoreSignals = true;
		empiSpawnTypeX->setValue(oldPosX);
		empiSpawnTypeY->setValue(oldPosY);
		this->ignoreSignals = false;
	}));

	placingSpawnLocation = false;
}

void EditorMovablePointPanel::onPasteIntoConfirmation(bool confirmed, std::shared_ptr<EditorMovablePoint> newEMP) {
	if (confirmed) {
		auto oldAnimatable = emp->getAnimatable();
		auto oldLoopAnimation = emp->getLoopAnimation();
		auto oldBaseSprite = emp->getBaseSprite();
		auto oldIsBullet = emp->getIsBullet();
		auto oldHitboxRadius = emp->getHitboxRadius();
		auto oldDespawnTime = emp->getDespawnTime();
		auto oldSpawnType = emp->getSpawnType();
		auto oldShadowTrailLifespan = emp->getShadowTrailLifespan();
		auto oldShadowTrailInterval = emp->getShadowTrailInterval();
		auto oldDamage = emp->getDamage();
		auto oldOnCollisionAction = emp->getOnCollisionAction();
		auto oldPierceResetTime = emp->getPierceResetTime();
		auto oldActions = emp->getActions();
		auto oldBulletModelID = emp->getBulletModelID();
		auto oldInheritRadius = emp->getInheritRadius();
		auto oldInheritDespawnTime = emp->getInheritDespawnTime();
		auto oldInheritShadowTrailInterval = emp->getInheritShadowTrailInterval();
		auto oldInheritShadowTrailLifespan = emp->getInheritShadowTrailLifespan();
		auto oldInheritAnimatables = emp->getInheritAnimatables();
		auto oldInheritDamage = emp->getInheritDamage();
		auto oldInheritSoundSettings = emp->getInheritSoundSettings();

		undoStack.execute(UndoableCommand([this, newEMP]() {
			emp->setAnimatable(newEMP->getAnimatable());
			emp->setLoopAnimation(newEMP->getLoopAnimation());
			emp->setBaseSprite(newEMP->getBaseSprite());
			emp->setIsBullet(newEMP->getIsBullet());
			emp->setHitboxRadius(newEMP->getHitboxRadius());
			emp->setDespawnTime(newEMP->getDespawnTime());
			emp->setSpawnType(newEMP->getSpawnType());
			emp->setShadowTrailLifespan(newEMP->getShadowTrailLifespan());
			emp->setShadowTrailInterval(newEMP->getShadowTrailInterval());
			emp->setDamage(newEMP->getDamage());
			emp->setOnCollisionAction(newEMP->getOnCollisionAction());
			emp->setPierceResetTime(newEMP->getPierceResetTime());
			if (newEMP->getBulletModelID() == -1) {
				emp->removeBulletModel();
			} else {
				emp->setBulletModel(this->levelPack.getBulletModel(newEMP->getBulletModelID()));
			}
			emp->setActions(newEMP->getActions());
			emp->setInheritRadius(newEMP->getInheritRadius(), levelPack);
			emp->setInheritDespawnTime(newEMP->getInheritDespawnTime(), levelPack);
			emp->setInheritShadowTrailInterval(newEMP->getInheritShadowTrailInterval(), levelPack);
			emp->setInheritShadowTrailLifespan(newEMP->getInheritShadowTrailLifespan(), levelPack);
			emp->setInheritAnimatables(newEMP->getInheritAnimatables(), levelPack);
			emp->setInheritDamage(newEMP->getInheritDamage(), levelPack);
			emp->setInheritSoundSettings(newEMP->getInheritSoundSettings(), levelPack);

			updateAllWidgetValues();

			onEMPModify.emit(this, this->emp);
		}, [this, oldAnimatable, oldLoopAnimation, oldBaseSprite, oldIsBullet, oldHitboxRadius, oldDespawnTime, oldSpawnType,
				oldShadowTrailLifespan, oldShadowTrailInterval, oldDamage, oldOnCollisionAction, oldPierceResetTime, oldBulletModelID, oldActions,
				oldInheritRadius, oldInheritDespawnTime, oldInheritShadowTrailInterval, oldInheritShadowTrailLifespan, oldInheritAnimatables,
				oldInheritDamage, oldInheritSoundSettings]() {
			emp->setAnimatable(oldAnimatable);
			emp->setLoopAnimation(oldLoopAnimation);
			emp->setBaseSprite(oldBaseSprite);
			emp->setIsBullet(oldIsBullet);
			emp->setHitboxRadius(oldHitboxRadius);
			emp->setDespawnTime(oldDespawnTime);
			emp->setSpawnType(oldSpawnType);
			emp->setShadowTrailLifespan(oldShadowTrailLifespan);
			emp->setShadowTrailInterval(oldShadowTrailInterval);
			emp->setDamage(oldDamage);
			emp->setOnCollisionAction(oldOnCollisionAction);
			emp->setPierceResetTime(oldPierceResetTime);
			if (oldBulletModelID == -1) {
				emp->removeBulletModel();
			} else {
				emp->setBulletModel(this->levelPack.getBulletModel(oldBulletModelID));
			}
			emp->setActions(oldActions);
			emp->setInheritRadius(oldInheritRadius, levelPack);
			emp->setInheritDespawnTime(oldInheritDespawnTime, levelPack);
			emp->setInheritShadowTrailInterval(oldInheritShadowTrailInterval, levelPack);
			emp->setInheritShadowTrailLifespan(oldInheritShadowTrailLifespan, levelPack);
			emp->setInheritAnimatables(oldInheritAnimatables, levelPack);
			emp->setInheritDamage(oldInheritDamage, levelPack);
			emp->setInheritSoundSettings(oldInheritSoundSettings, levelPack);

			updateAllWidgetValues();

			onEMPModify.emit(this, this->emp);
		}));
	}
}
