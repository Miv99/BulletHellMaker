#include "EditorMovablePointPanel.h"
#include "EditorMovablePointActionPanel.h"

const std::string EditorMovablePointPanel::PROPERTIES_TAB_NAME = "MP Properties";
const std::string EditorMovablePointPanel::MOVEMENT_TAB_NAME = "MP Movement";

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
	spawnTypePositionMarkerPlacer = SingleMarkerPlacer::create(*(mainEditorWindow.getWindow()), clipboard);
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
		empiHitboxRadius = EditBox::create();
		
		empiDespawnTimeLabel = tgui::Label::create();
		// Max value is sum of time taken for every EMPA in empiActions
		empiDespawnTime = std::make_shared<SliderWithEditBox>();

		empiSpawnTypeLabel = tgui::Label::create();
		// Entry ID is from getID()
		empiSpawnType = tgui::ComboBox::create();
		empiSpawnTypeTimeLabel = tgui::Label::create();
		empiSpawnTypeTime = EditBox::create();
		empiSpawnTypeXLabel = tgui::Label::create();
		empiSpawnTypeX = EditBox::create();
		empiSpawnTypeYLabel = tgui::Label::create();
		empiSpawnTypeY = EditBox::create();
		empiSpawnLocationManualSet = tgui::Button::create();

		empiShadowTrailLifespanLabel = tgui::Label::create();
		empiShadowTrailLifespan = EditBox::create();
		empiShadowTrailIntervalLabel = tgui::Label::create();
		empiShadowTrailInterval = EditBox::create();

		empiDamageLabel = tgui::Label::create();
		empiDamage = EditBox::create();

		empiOnCollisionActionLabel = tgui::Label::create();
		// Entry ID obtained from getID()
		empiOnCollisionAction = tgui::ComboBox::create();

		empiPierceResetTimeLabel = tgui::Label::create();
		empiPierceResetTime = EditBox::create();

		empiSoundSettingsLabel = tgui::Label::create();
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
		empiInheritPierceResetTime = tgui::CheckBox::create();
		empiInheritSoundSettings = tgui::CheckBox::create();

		empiAnimatableLabel->setToolTip(createToolTip("The default sprite/animation for this movable point."));
		empiLoopAnimation->setToolTip(createToolTip("If this is checked, the default animation will loop indefinitely."));
		empiBaseSpriteLabel->setToolTip(createToolTip("The fallback sprite after the animation finishes. Only used if the default animation does not loop."));
		isBullet->setToolTip(createToolTip("If this is checked, this movable point will be a bullet that is able to damage entities. If the bullet originated from \
a player, it can damage only enemies. If the bullet originated from an enemy, it can damage only players."));
		empiHitboxRadiusLabel->setToolTip(createToolTip("The radius of this bullet. Only used if this movable point is a bullet."));
		empiDespawnTimeLabel->setToolTip(createToolTip("Number of seconds after being spawned that this movable point despawns. When a movable point despawns, every movable point \
attached to it also despawns, so this effectively despawns all movable points in the movable point attachment tree with this one as the root."));
		empiSpawnTypeLabel->setToolTip(createToolTip("Determines how this movable point will be spawned.\n\n\
\"Relative to map origin\" - Spawns at some absolute position\n\n\
\"Detached, relative to parent\" - Spawns relative to this movable point's parent\n\n\
\"Attached, relative to parent\" - Spawns relative to this movable point's parent and moves relative to its parent until this movable point does a detach movement action"));
		empiSpawnTypeTimeLabel->setToolTip(createToolTip("Number of seconds after this movable point's parent spawns that this movable point spawns. If \
this movable point is the main movable point of its attack (the root of the attack's movable point tree), this value will be fixed to 0 so this movable point spawns as soon \
as the attack is executed."));
		empiSpawnTypeXLabel->setToolTip(createToolTip("The x-position for this movable point's spawn. See \"Spawn type\" for how this position will be interpreted."));
		empiSpawnTypeYLabel->setToolTip(createToolTip("The y-position for this movable point's spawn. See \"Spawn type\" for how this position will be interpreted."));
		empiSpawnLocationManualSet->setToolTip(createToolTip("Opens a map to help visualize and set this movable point's spawn position."));
		empiShadowTrailLifespanLabel->setToolTip(createToolTip("Number of seconds each of this movable point's shadows last. Shadows are purely visual and create a movement trail."));
		empiShadowTrailIntervalLabel->setToolTip(createToolTip("Number of seconds between the creation of each shadow. Shadows are purely visual and create a movement trail."));
		empiDamageLabel->setToolTip(createToolTip("Damage dealt to an enemy/player on contact with this bullet. Only used if this movable point is a bullet. Value will be rounded to the nearest integer."));
		empiOnCollisionActionLabel->setToolTip(createToolTip("Determines how this bullet will act on contact with an enemy/player.\n\n\
\"Destroy self only\" - This bullet becomes invisible and intangible upon hitting an enemy/player but will still continue following its movement actions until it despawns. \
This means any movable points attached to this bullet when it collided with an enemy/player will behave as if nothing happened. \n\n\
\"Destroy self and attached children\" - This bullet, and every movable point attached to it, despawns upon hitting an enemy/player. When a movable point despawns, every movable point \
attached to it also despawns, so this effectively despawns all movable points in the movable point attachment tree with this one as the root. \n\n\
\"Pierce players/enemies\" - This bullet does not do anything special upon hitting an enemy/player, so it is able to hit multiple enemies/players multiple times. \
Each enemy/player can be hit at most every \"Pierce reset time\" seconds by the same bullet. Players also have a custom invulnerability time every time they take damage, \
so this should be considered as well."));
		empiPierceResetTimeLabel->setToolTip(createToolTip("Minimum number of seconds after this bullet hits a player/enemy that it can hit the same player/enemy again. \
Players also have a custom invulnerability time every time they take damage, so this should be considered as well."));
		empiSoundSettingsLabel->setToolTip(createToolTip("Settings for the sound to be played when this movable point is spawned."));
		empiBulletModelLabel->setToolTip(createToolTip("The movable point model that this movable point will use. This is purely for convenience by allowing this movable point to \
use the radius, despawn time, shadow settings, sprites and animations, damage, and/or sound settings of some user-defined model such that whenever the model is updated, this movable \
point will update only the values it wants to inherit to match the model."));
		empiInheritRadius->setToolTip(createToolTip("If this is checked, this movable point will use its model's radius."));
		empiInheritDespawnTime->setToolTip(createToolTip("If this is checked, this movable point will use its model's despawn time."));
		empiInheritShadowTrailInterval->setToolTip(createToolTip("If this is checked, this movable point will use its model's shadow trail interval."));
		empiInheritShadowTrailLifespan->setToolTip(createToolTip("If this is checked, this movable point will use its model's shadow trail lifespan."));
		empiInheritAnimatables->setToolTip(createToolTip("If this is checked, this movable point will use its model's sprites and animations."));
		empiInheritDamage->setToolTip(createToolTip("If this is checked, this movable point will use its model's damage."));
		empiInheritPierceResetTime->setToolTip(createToolTip("If this is checked, this movable point will use its model's pierce reset time."));
		empiInheritSoundSettings->setToolTip(createToolTip("If this is checked, this movable point will use its model's sound settings."));

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
		empiInheritPierceResetTime->setTextSize(TEXT_SIZE);
		empiInheritSoundSettings->setTextSize(TEXT_SIZE);
		empiSoundSettingsLabel->setTextSize(TEXT_SIZE);

		id->setText("Movable point ID " + std::to_string(emp->getID()));
		empiHitboxRadiusLabel->setText("Hitbox radius");
		empiAnimatableLabel->setText("Sprite/Animation");
		empiBaseSpriteLabel->setText("Base sprite");
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
		empiBulletModelLabel->setText("Movable point model");
		empiInheritRadius->setText("Inherit radius");
		empiInheritDespawnTime->setText("Inherit despawn time");
		empiInheritShadowTrailInterval->setText("Inherit shadow trail interval");
		empiInheritShadowTrailLifespan->setText("Inherit shadow trail lifespan");
		empiInheritAnimatables->setText("Inherit animatables");
		empiInheritDamage->setText("Inherit damage");
		empiInheritPierceResetTime->setText("Inherit pierce reset time");
		empiInheritSoundSettings->setText("Inherit sound settings");
		empiSoundSettingsLabel->setText("Spawn sound");

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
				empiOnCollisionAction->setEnabled(this->emp->getIsBullet());
				empiHitboxRadius->setEnabled((!this->emp->getInheritRadius() || this->emp->getBulletModelID() < 0) && this->emp->getIsBullet());
				empiDamage->setEnabled((!this->emp->getInheritDamage() || this->emp->getBulletModelID() < 0) && this->emp->getIsBullet());
				empiPierceResetTimeLabel->setVisible(this->emp->getOnCollisionAction() == PIERCE_ENTITY && this->emp->getIsBullet());
				empiPierceResetTime->setVisible(this->emp->getOnCollisionAction() == PIERCE_ENTITY && this->emp->getIsBullet());
				this->ignoreSignals = false;

				onEMPModify.emit(this, this->emp);
			}, [this, oldValue]() {
				this->emp->setIsBullet(oldValue);

				this->ignoreSignals = true;
				isBullet->setChecked(oldValue);
				empiOnCollisionAction->setEnabled(this->emp->getIsBullet());
				empiHitboxRadius->setEnabled((!this->emp->getInheritRadius() || this->emp->getBulletModelID() < 0) && this->emp->getIsBullet());
				empiDamage->setEnabled((!this->emp->getInheritDamage() || this->emp->getBulletModelID() < 0) && this->emp->getIsBullet());
				empiPierceResetTimeLabel->setVisible(this->emp->getOnCollisionAction() == PIERCE_ENTITY && this->emp->getIsBullet());
				empiPierceResetTime->setVisible(this->emp->getOnCollisionAction() == PIERCE_ENTITY && this->emp->getIsBullet());
				this->ignoreSignals = false;

				onEMPModify.emit(this, this->emp);
			}));
		});
		empiHitboxRadius->connect("ValueChanged", [this](std::string value) {
			if (this->ignoreSignals) {
				return;
			}

			std::string oldValue = this->emp->getRawHitboxRadius();
			undoStack.execute(UndoableCommand([this, value]() {
				this->emp->setHitboxRadius(value);
				this->ignoreSignals = true;
				empiHitboxRadius->setText(value);
				this->ignoreSignals = false;
				onEMPModify.emit(this, this->emp);
			}, [this, oldValue]() {
				this->emp->setHitboxRadius(oldValue);
				this->ignoreSignals = true;
				empiHitboxRadius->setText(oldValue);
				this->ignoreSignals = false;
				onEMPModify.emit(this, this->emp);
			}));
		});
		empiDespawnTime->connect("ValueChanged", [this](float value) {
			if (this->ignoreSignals) {
				return;
			}

			float oldValue = this->emp->getDespawnTime();
			undoStack.execute(UndoableCommand([this, value]() {
				this->emp->setDespawnTime(value);
				this->ignoreSignals = true;
				empiDespawnTime->setValue(value);
				this->ignoreSignals = false;
				onEMPModify.emit(this, this->emp);
			}, [this, oldValue]() {
				this->emp->setDespawnTime(oldValue);
				this->ignoreSignals = true;
				empiDespawnTime->setValue(oldValue);
				this->ignoreSignals = false;
				onEMPModify.emit(this, this->emp);
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
					this->emp->setSpawnType(std::make_shared<SpecificGlobalEMPSpawn>(empiSpawnTypeTime->getText(), empiSpawnTypeX->getText(), empiSpawnTypeY->getText()));
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
					this->emp->setSpawnType(std::make_shared<EntityRelativeEMPSpawn>(empiSpawnTypeTime->getText(), empiSpawnTypeX->getText(), empiSpawnTypeY->getText()));
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
					this->emp->setSpawnType(std::make_shared<EntityAttachedEMPSpawn>(empiSpawnTypeTime->getText(), empiSpawnTypeX->getText(), empiSpawnTypeY->getText()));
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
		empiSpawnTypeTime->connect("ValueChanged", [this](std::string value) {
			if (ignoreSignals) {
				return;
			}

			std::string oldValue = this->emp->getSpawnType()->getRawTime();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setSpawnTypeTime(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSpawnTypeTime->setText(this->emp->getSpawnType()->getRawTime());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->setSpawnTypeTime(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSpawnTypeTime->setText(this->emp->getSpawnType()->getRawTime());
				ignoreSignals = false;
			}));
		});
		empiSpawnTypeX->connect("ValueChanged", [this](std::string value) {
			if (ignoreSignals) {
				return;
			}

			std::string oldValue = this->emp->getSpawnType()->getRawX();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->getSpawnType()->setX(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSpawnTypeX->setText(this->emp->getSpawnType()->getRawX());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->getSpawnType()->setX(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSpawnTypeX->setText(this->emp->getSpawnType()->getRawX());
				ignoreSignals = false;
			}));
		});
		empiSpawnTypeY->connect("ValueChanged", [this](std::string value) {
			if (ignoreSignals) {
				return;
			}

			std::string oldValue = this->emp->getSpawnType()->getRawY();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->getSpawnType()->setY(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSpawnTypeY->setText(this->emp->getSpawnType()->getRawY());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->getSpawnType()->setY(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiSpawnTypeY->setText(this->emp->getSpawnType()->getRawY());
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
		empiShadowTrailLifespan->connect("ValueChanged", [this](std::string value) {
			if (ignoreSignals) {
				return;
			}

			std::string oldValue = this->emp->getRawShadowTrailLifespan();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setShadowTrailLifespan(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiShadowTrailLifespan->setText(this->emp->getRawShadowTrailLifespan());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->setShadowTrailLifespan(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiShadowTrailLifespan->setText(this->emp->getRawShadowTrailLifespan());
				ignoreSignals = false;
			}));
		});
		empiShadowTrailInterval->connect("ValueChanged", [this](std::string value) {
			if (ignoreSignals) {
				return;
			}

			std::string oldValue = this->emp->getRawShadowTrailInterval();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setShadowTrailInterval(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiShadowTrailInterval->setText(this->emp->getRawShadowTrailInterval());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->setShadowTrailInterval(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiShadowTrailInterval->setText(this->emp->getRawShadowTrailInterval());
				ignoreSignals = false;
			}));
		});
		empiDamage->connect("ValueChanged", [this](std::string value) {
			if (ignoreSignals) {
				return;
			}

			std::string oldValue = this->emp->getRawDamage();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setDamage(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiDamage->setText(this->emp->getRawDamage());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->setDamage(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiDamage->setText(this->emp->getRawDamage());
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
				empiPierceResetTimeLabel->setVisible(action == PIERCE_ENTITY && this->emp->getIsBullet());
				empiPierceResetTime->setVisible(action == PIERCE_ENTITY && this->emp->getIsBullet());
				ignoreSignals = false;
			},
				[this, oldAction]() {
				this->emp->setOnCollisionAction(oldAction);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiOnCollisionAction->setSelectedItemById(getID(oldAction));
				empiPierceResetTimeLabel->setVisible(oldAction == PIERCE_ENTITY && this->emp->getIsBullet());
				empiPierceResetTime->setVisible(oldAction == PIERCE_ENTITY && this->emp->getIsBullet());
				ignoreSignals = false;
			}));
		});
		empiPierceResetTime->connect("ValueChanged", [this](std::string value) {
			if (ignoreSignals) {
				return;
			}

			std::string oldValue = this->emp->getRawPierceResetTime();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setPierceResetTime(value);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiPierceResetTime->setText(this->emp->getRawPierceResetTime());
				ignoreSignals = false;
			},
				[this, oldValue]() {
				this->emp->setPierceResetTime(oldValue);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiPierceResetTime->setText(this->emp->getRawPierceResetTime());
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
			std::string radius = this->emp->getRawHitboxRadius();
			float despawnTime = this->emp->getDespawnTime();
			std::string interval = this->emp->getRawShadowTrailInterval();
			std::string lifespan = this->emp->getRawShadowTrailLifespan();
			Animatable animatable = this->emp->getAnimatable();
			Animatable baseSprite = this->emp->getBaseSprite();
			bool loopAnimation = this->emp->getLoopAnimation();
			std::string damage = this->emp->getRawDamage();
			std::string pierceResetTime = this->emp->getRawPierceResetTime();
			SoundSettings sound = this->emp->getSoundSettings();
			undoStack.execute(UndoableCommand(
				[this, bulletModelID, radius, despawnTime, interval, lifespan, animatable, baseSprite, loopAnimation, damage, pierceResetTime, sound]() {
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
				empiHitboxRadius->setText(this->emp->getRawHitboxRadius());
				empiDespawnTime->setValue(this->emp->getDespawnTime());
				empiShadowTrailLifespan->setText(this->emp->getRawShadowTrailLifespan());
				empiShadowTrailInterval->setText(this->emp->getRawShadowTrailInterval());
				empiAnimatable->setValue(this->emp->getAnimatable());
				empiLoopAnimation->setChecked(this->emp->getLoopAnimation());
				empiBaseSprite->setValue(this->emp->getAnimatable());
				empiDamage->setText(this->emp->getRawDamage());
				empiPierceResetTime->setText(this->emp->getRawPierceResetTime());
				empiSoundSettings->initSettings(this->emp->getSoundSettings());

				empiLoopAnimation->setVisible(!this->emp->getAnimatable().isSprite());
				empiBaseSprite->setVisible(!this->emp->getLoopAnimation() && !this->emp->getAnimatable().isSprite());
				empiBaseSpriteLabel->setVisible(empiBaseSprite->isVisible());
				empiPierceResetTimeLabel->setVisible((this->emp->getOnCollisionAction() == PIERCE_ENTITY) && this->emp->getIsBullet());
				empiPierceResetTime->setVisible((this->emp->getOnCollisionAction() == PIERCE_ENTITY) && this->emp->getIsBullet());

				empiHitboxRadius->setEnabled((!this->emp->getInheritRadius() || this->emp->getBulletModelID() < 0) && this->emp->getIsBullet());
				empiDespawnTime->setEnabled(!this->emp->getInheritDespawnTime() || this->emp->getBulletModelID() < 0);
				empiShadowTrailInterval->setEnabled(!this->emp->getInheritShadowTrailInterval() || this->emp->getBulletModelID() < 0);
				empiShadowTrailLifespan->setEnabled(!this->emp->getInheritShadowTrailLifespan() || this->emp->getBulletModelID() < 0);
				empiAnimatable->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
				empiLoopAnimation->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
				empiBaseSprite->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
				empiDamage->setEnabled((!this->emp->getInheritDamage() || this->emp->getBulletModelID() < 0) && this->emp->getIsBullet());
				empiPierceResetTime->setEnabled((!this->emp->getInheritPierceResetTime() || this->emp->getBulletModelID() < 0) && this->emp->getIsBullet());
				empiSoundSettings->setEnabled(!this->emp->getInheritSoundSettings() || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			},
				[this, oldBulletModelID, radius, despawnTime, interval, lifespan, animatable, baseSprite, loopAnimation, damage, pierceResetTime, sound]() {
				this->emp->setHitboxRadius(radius);
				this->emp->setDespawnTime(despawnTime);
				this->emp->setShadowTrailLifespan(lifespan);
				this->emp->setShadowTrailInterval(interval);
				this->emp->setAnimatable(animatable);
				this->emp->setLoopAnimation(loopAnimation);
				this->emp->setBaseSprite(baseSprite);
				this->emp->setDamage(damage);
				this->emp->setPierceResetTime(pierceResetTime);
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
				empiHitboxRadius->setText(this->emp->getRawHitboxRadius());
				empiDespawnTime->setValue(this->emp->getDespawnTime());
				empiShadowTrailLifespan->setText(this->emp->getRawShadowTrailLifespan());
				empiShadowTrailInterval->setText(this->emp->getRawShadowTrailInterval());
				empiAnimatable->setValue(this->emp->getAnimatable());
				empiLoopAnimation->setChecked(this->emp->getLoopAnimation());
				empiBaseSprite->setValue(this->emp->getAnimatable());
				empiDamage->setText(this->emp->getRawDamage());
				empiPierceResetTime->setText(this->emp->getRawPierceResetTime());
				empiSoundSettings->initSettings(this->emp->getSoundSettings());

				empiLoopAnimation->setVisible(!this->emp->getAnimatable().isSprite());
				empiBaseSprite->setVisible(!this->emp->getLoopAnimation() && !this->emp->getAnimatable().isSprite());
				empiBaseSpriteLabel->setVisible(empiBaseSprite->isVisible());
				empiPierceResetTimeLabel->setVisible((this->emp->getOnCollisionAction() == PIERCE_ENTITY) && this->emp->getIsBullet());
				empiPierceResetTime->setVisible((this->emp->getOnCollisionAction() == PIERCE_ENTITY) && this->emp->getIsBullet());

				empiHitboxRadius->setEnabled(!this->emp->getInheritRadius() || this->emp->getBulletModelID() < 0);
				empiDespawnTime->setEnabled(!this->emp->getInheritDespawnTime() || this->emp->getBulletModelID() < 0);
				empiShadowTrailInterval->setEnabled(!this->emp->getInheritShadowTrailInterval() || this->emp->getBulletModelID() < 0);
				empiShadowTrailLifespan->setEnabled(!this->emp->getInheritShadowTrailLifespan() || this->emp->getBulletModelID() < 0);
				empiAnimatable->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
				empiLoopAnimation->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
				empiBaseSprite->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
				empiDamage->setEnabled((!this->emp->getInheritDamage() || this->emp->getBulletModelID() < 0) && this->emp->getIsBullet());
				empiPierceResetTime->setEnabled((!this->emp->getInheritPierceResetTime() || this->emp->getBulletModelID() < 0) && this->emp->getIsBullet());
				empiSoundSettings->setEnabled(!this->emp->getInheritSoundSettings() || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			}));
		});
		empiInheritRadius->connect("Changed", [this](bool value) {
			if (ignoreSignals) {
				return;
			}

			bool oldValue = this->emp->getInheritRadius();
			std::string oldInheritValue = this->emp->getRawHitboxRadius();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setInheritRadius(value, this->levelPack);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritRadius->setChecked(this->emp->getInheritRadius());
				empiHitboxRadius->setText(this->emp->getRawHitboxRadius());
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
				empiHitboxRadius->setText(this->emp->getRawHitboxRadius());
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
			std::string oldInheritValue = this->emp->getRawShadowTrailInterval();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setInheritShadowTrailInterval(value, this->levelPack);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritShadowTrailInterval->setChecked(this->emp->getInheritShadowTrailInterval());
				empiShadowTrailInterval->setText(this->emp->getRawShadowTrailInterval());
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
				empiShadowTrailInterval->setText(this->emp->getRawShadowTrailInterval());
				empiShadowTrailInterval->setEnabled(!oldValue || this->emp->getBulletModelID() < 0);
				ignoreSignals = false;
			}));
		});
		empiInheritShadowTrailLifespan->connect("Changed", [this](bool value) {
			if (ignoreSignals) {
				return;
			}

			bool oldValue = this->emp->getInheritShadowTrailLifespan();
			std::string oldInheritValue = this->emp->getRawShadowTrailLifespan();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setInheritShadowTrailLifespan(value, this->levelPack);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritShadowTrailLifespan->setChecked(this->emp->getInheritShadowTrailLifespan());
				empiShadowTrailLifespan->setText(this->emp->getRawShadowTrailLifespan());
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
				empiShadowTrailLifespan->setText(this->emp->getRawShadowTrailLifespan());
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
			std::string oldInheritValue = this->emp->getRawDamage();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setInheritDamage(value, this->levelPack);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritDamage->setChecked(this->emp->getInheritDamage());
				empiDamage->setText(this->emp->getRawDamage());
				empiDamage->setEnabled((!value || this->emp->getBulletModelID() < 0) && this->emp->getIsBullet());
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
				empiDamage->setText(this->emp->getRawDamage());
				empiDamage->setEnabled((!oldValue || this->emp->getBulletModelID() < 0) && this->emp->getIsBullet());
				ignoreSignals = false;
			}));
		});
		empiInheritPierceResetTime->connect("Changed", [this](bool value) {
			if (ignoreSignals) {
				return;
			}

			bool oldValue = this->emp->getInheritPierceResetTime();
			std::string oldInheritValue = this->emp->getRawPierceResetTime();
			undoStack.execute(UndoableCommand(
				[this, value]() {
				this->emp->setInheritPierceResetTime(value, this->levelPack);
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritPierceResetTime->setChecked(this->emp->getInheritPierceResetTime());
				empiPierceResetTime->setText(this->emp->getRawPierceResetTime());
				empiPierceResetTime->setEnabled((!value || this->emp->getBulletModelID() < 0) && this->emp->getIsBullet());
				ignoreSignals = false;
			},
				[this, oldValue, oldInheritValue]() {
				this->emp->setInheritPierceResetTime(oldValue, this->levelPack);
				if (!oldValue) {
					this->emp->setPierceResetTime(oldInheritValue);
				}
				onEMPModify.emit(this, this->emp);

				ignoreSignals = true;
				empiInheritPierceResetTime->setChecked(this->emp->getInheritPierceResetTime());
				empiPierceResetTime->setText(this->emp->getRawPierceResetTime());
				empiPierceResetTime->setEnabled((!oldValue || this->emp->getBulletModelID() < 0) && this->emp->getIsBullet());
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

		empiBulletModelLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiBaseSprite) + GUI_PADDING_Y * 2);
		empiBulletModel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiBulletModelLabel) + GUI_LABEL_PADDING_Y);
		empiInheritRadius->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiBulletModel) + GUI_LABEL_PADDING_Y);
		empiInheritDespawnTime->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritRadius) + GUI_LABEL_PADDING_Y);
		empiInheritShadowTrailInterval->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritDespawnTime) + GUI_LABEL_PADDING_Y);
		empiInheritShadowTrailLifespan->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritShadowTrailInterval) + GUI_LABEL_PADDING_Y);
		empiInheritAnimatables->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritShadowTrailLifespan) + GUI_LABEL_PADDING_Y);
		empiInheritDamage->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritAnimatables) + GUI_LABEL_PADDING_Y);
		empiInheritPierceResetTime->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritDamage) + GUI_LABEL_PADDING_Y);
		empiInheritSoundSettings->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritPierceResetTime) + GUI_LABEL_PADDING_Y);

		isBullet->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiInheritSoundSettings) + GUI_PADDING_Y * 2);
		empiHitboxRadiusLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(isBullet) + GUI_PADDING_Y);
		empiHitboxRadius->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiHitboxRadiusLabel) + GUI_LABEL_PADDING_Y);
		empiDamageLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiHitboxRadius) + GUI_PADDING_Y * 2);
		empiDamage->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiDamageLabel) + GUI_LABEL_PADDING_Y);
		empiOnCollisionActionLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiDamage) + GUI_PADDING_Y);
		empiOnCollisionAction->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiOnCollisionActionLabel) + GUI_LABEL_PADDING_Y);
		empiPierceResetTimeLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiOnCollisionAction) + GUI_PADDING_Y);
		empiPierceResetTime->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiPierceResetTimeLabel) + GUI_LABEL_PADDING_Y);

		empiDespawnTimeLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiPierceResetTime) + GUI_PADDING_Y * 2);
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
		empiSoundSettingsLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiSpawnLocationManualSet) + GUI_PADDING_Y * 2);
		empiSoundSettings->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiSoundSettingsLabel) + GUI_LABEL_PADDING_Y);
		empiShadowTrailLifespanLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiSoundSettings) + GUI_PADDING_Y * 2);
		empiShadowTrailLifespan->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiShadowTrailLifespanLabel) + GUI_LABEL_PADDING_Y);
		empiShadowTrailIntervalLabel->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiShadowTrailLifespan) + GUI_PADDING_Y);
		empiShadowTrailInterval->setPosition(tgui::bindLeft(id), tgui::bindBottom(empiShadowTrailIntervalLabel) + GUI_LABEL_PADDING_Y);

		tgui::Layout fillWidth = tgui::bindWidth(properties) - GUI_PADDING_X * 2;
		empiLoopAnimation->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiAnimatable->setSize(fillWidth, 0);
		empiBaseSprite->setSize(fillWidth, 0);
		empiAnimatable->setAnimatablePictureSize(fillWidth, tgui::bindMin(tgui::bindWidth(properties) - GUI_PADDING_X * 2, 120));
		empiBaseSprite->setAnimatablePictureSize(fillWidth, tgui::bindMin(tgui::bindWidth(properties) - GUI_PADDING_X * 2, 120));
		empiHitboxRadius->setSize(fillWidth, TEXT_BOX_HEIGHT);
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
		empiBulletModel->setSize(fillWidth, TEXT_BOX_HEIGHT);
		empiInheritRadius->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiInheritDespawnTime->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiInheritShadowTrailInterval->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiInheritShadowTrailLifespan->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiInheritAnimatables->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiInheritDamage->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		empiInheritPierceResetTime->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
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
		properties->add(empiSoundSettingsLabel);
		properties->add(empiSoundSettings);
		properties->add(empiBulletModelLabel);
		properties->add(empiBulletModel);
		properties->add(empiInheritRadius);
		properties->add(empiInheritDespawnTime);
		properties->add(empiInheritShadowTrailInterval);
		properties->add(empiInheritShadowTrailLifespan);
		properties->add(empiInheritAnimatables);
		properties->add(empiInheritDamage);
		properties->add(empiInheritPierceResetTime);
		properties->add(empiInheritSoundSettings);

		properties->connect("SizeChanged", [this](sf::Vector2f newSize) {
			// This is here because of some random bug with SoundSettingsGroup
			empiSoundSettings->setSize(newSize.x - GUI_PADDING_X * 2, 0);

			spawnTypePositionMarkerPlacerFinishEditing->setPosition(newSize.x - spawnTypePositionMarkerPlacerFinishEditing->getSize().x, newSize.y - spawnTypePositionMarkerPlacerFinishEditing->getSize().y);
		});

		tabs->addTab(PROPERTIES_TAB_NAME, properties);
	}
	{
		// Movement tab
		movementEditorPanel = EMPABasedMovementEditorPanel::create(mainEditorWindow, clipboard);
		movementEditorPanel->connect("EMPAListModified", [this](std::vector<std::shared_ptr<EMPAction>> newActions, float newSumOfDurations) {
			// This shouldn't be undoable here because it's already undoable from EMPABasedMovementEditorPanel.
			// Note: Setting the limits of a SliderWithEditBox to some number and then setting it back does not
			// revert the SliderWithEditBox's value
			this->emp->setActions(newActions);
			// Max time for despawn time is sum of actions' durations
			this->empiDespawnTime->setMax(newSumOfDurations);

			onEMPModify.emit(this, this->emp);
		});
		movementEditorPanel->setActions(this->emp->getActions());
		empiDespawnTime->setMax(movementEditorPanel->getSumOfDurations());
		tabs->addTab(MOVEMENT_TAB_NAME, movementEditorPanel, false, false);
	}

	symbolTableEditorWindow = tgui::ChildWindow::create();
	symbolTableEditor = ValueSymbolTableEditor::create(false, false);
	symbolTableEditorWindow->setKeepInParent(false);
	symbolTableEditorWindow->add(symbolTableEditor);
	symbolTableEditorWindow->setSize("50%", "50%");
	symbolTableEditorWindow->setTitle("Movable Point ID " + std::to_string(emp->getID()) + " Variables");
	symbolTableEditor->connect("ValueChanged", [this](ValueSymbolTable table) {
		this->emp->setSymbolTable(table);
		onChange(table);
	});
}

EditorMovablePointPanel::~EditorMovablePointPanel() {
	levelPack.getOnChange()->sink().disconnect<EditorMovablePointPanel, &EditorMovablePointPanel::onLevelPackChange>(this);
	mainEditorWindow.getGui()->remove(symbolTableEditorWindow);
}

std::pair<std::shared_ptr<CopiedObject>, std::string> EditorMovablePointPanel::copyFrom() {
	// Can't copy this widget
	return std::make_pair(nullptr, "");
}

std::string EditorMovablePointPanel::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	// Same functionality as paste2Into()
	return paste2Into(pastedObject);
}

std::string EditorMovablePointPanel::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	// Paste the first copied EditorMovablePoint to override emp's properties
	auto derived = std::static_pointer_cast<CopiedEditorMovablePoint>(pastedObject);
	if (derived) {
		std::shared_ptr<EditorMovablePoint> copiedEMP = derived->getEMP();

		mainEditorWindow.promptConfirmation("Overwrite this movable point's properties with the copied movable point's properties? This will not change this movable point's children.", copiedEMP)->sink()
			.connect<EditorMovablePointPanel, &EditorMovablePointPanel::onPasteIntoConfirmation>(this);
	}
	return "";
}

bool EditorMovablePointPanel::handleEvent(sf::Event event) {
	if (symbolTableEditorWindow->isFocused()) {
		return symbolTableEditor->handleEvent(event);
	} else if (tabs->handleEvent(event)) {
		return true;
	} else if (placingSpawnLocation) {
		if (spawnTypePositionMarkerPlacer->handleEvent(event)) {
			return true;
		}

		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
			finishEditingSpawnTypePosition();
			return true;
		}
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
		} else if (event.key.code == sf::Keyboard::V) {
			mainEditorWindow.getGui()->add(symbolTableEditorWindow);
			return true;
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

void EditorMovablePointPanel::propagateChangesToChildren() {
	symbolTableEditor->setSymbolTablesHierarchy(symbolTables);

	// movementEditorPanel acts as just a middleman between this widget and child widgets that
	// use emp's ValueSymbolTable
	movementEditorPanel->propagateChangesToChildren();
}

ValueSymbolTable EditorMovablePointPanel::getLevelPackObjectSymbolTable() {
	return emp->getSymbolTable();
}

void EditorMovablePointPanel::updateAllWidgetValues() {
	// Update widgets whose values can be changed by the player
	ignoreSignals = true;
	empiAnimatable->setValue(emp->getAnimatable());
	empiLoopAnimation->setChecked(emp->getLoopAnimation());
	empiBaseSprite->setValue(emp->getAnimatable());
	isBullet->setChecked(emp->getIsBullet());
	empiHitboxRadius->setText(emp->getRawHitboxRadius());
	empiDespawnTime->setValue(emp->getDespawnTime());
	empiSpawnType->setSelectedItemById(getID(emp->getSpawnType()));
	// empiSpawnTypeTime should always display 0 if the EMP is the main EMP of its EditorAttack
	// because it will always be spawned instantly
	empiSpawnTypeTime->setText(emp->isMainEMP() ? "0" : emp->getSpawnType()->getRawTime());
	empiSpawnTypeX->setText(emp->getSpawnType()->getRawX());
	empiSpawnTypeY->setText(emp->getSpawnType()->getRawY());
	empiShadowTrailLifespan->setText(emp->getRawShadowTrailLifespan());
	empiShadowTrailInterval->setText(emp->getRawShadowTrailInterval());
	empiDamage->setText(emp->getRawDamage());
	empiOnCollisionAction->setSelectedItemById(getID(emp->getOnCollisionAction()));
	empiPierceResetTime->setText(emp->getRawPierceResetTime());
	empiSoundSettings->initSettings(emp->getSoundSettings());
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
	empiInheritPierceResetTime->setChecked(emp->getInheritPierceResetTime());
	empiInheritSoundSettings->setChecked(emp->getInheritSoundSettings());

	empiSpawnTypeTime->setEnabled(!emp->isMainEMP());
	empiOnCollisionAction->setEnabled(emp->getIsBullet());
	empiHitboxRadius->setEnabled((!emp->getInheritRadius() || this->emp->getBulletModelID() < 0) && emp->getIsBullet());
	empiDespawnTime->setEnabled(!emp->getInheritDespawnTime() || this->emp->getBulletModelID() < 0);
	empiShadowTrailInterval->setEnabled(!emp->getInheritShadowTrailInterval() || this->emp->getBulletModelID() < 0);
	empiShadowTrailLifespan->setEnabled(!emp->getInheritShadowTrailLifespan() || this->emp->getBulletModelID() < 0);
	empiAnimatable->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
	empiLoopAnimation->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
	empiBaseSprite->setEnabled(!this->emp->getInheritAnimatables() || this->emp->getBulletModelID() < 0);
	empiDamage->setEnabled((!emp->getInheritDamage() || this->emp->getBulletModelID() < 0) && emp->getIsBullet());
	empiPierceResetTime->setEnabled((!emp->getInheritPierceResetTime() || this->emp->getBulletModelID() < 0) && emp->getIsBullet());
	empiSoundSettings->setEnabled(!emp->getInheritSoundSettings() || this->emp->getBulletModelID() < 0);

	empiLoopAnimation->setVisible(!emp->getAnimatable().isSprite());
	empiBaseSprite->setVisible(!emp->getLoopAnimation() && !emp->getAnimatable().isSprite());
	empiBaseSpriteLabel->setVisible(empiBaseSprite->isVisible());
	empiPierceResetTimeLabel->setVisible((emp->getOnCollisionAction() == PIERCE_ENTITY) && emp->getIsBullet());
	empiPierceResetTime->setVisible((emp->getOnCollisionAction() == PIERCE_ENTITY) && emp->getIsBullet());
	ignoreSignals = false;
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
		emp->getSpawnType()->setX(formatNum(newPos.x));
		emp->getSpawnType()->setY(formatNum(newPos.y));
		onEMPModify.emit(this, this->emp);

		this->ignoreSignals = true;
		empiSpawnTypeX->setText(formatNum(newPos.x));
		empiSpawnTypeY->setText(formatNum(newPos.y));
		this->ignoreSignals = false;
	},
		[this, oldPosX, oldPosY]() {
		emp->getSpawnType()->setX(formatNum(oldPosX));
		emp->getSpawnType()->setY(formatNum(oldPosY));
		onEMPModify.emit(this, this->emp);

		this->ignoreSignals = true;
		empiSpawnTypeX->setText(formatNum(oldPosX));
		empiSpawnTypeY->setText(formatNum(oldPosY));
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
		auto oldHitboxRadius = emp->getRawHitboxRadius();
		auto oldDespawnTime = emp->getDespawnTime();
		auto oldSpawnType = emp->getSpawnType();
		auto oldShadowTrailLifespan = emp->getRawShadowTrailLifespan();
		auto oldShadowTrailInterval = emp->getRawShadowTrailInterval();
		auto oldDamage = emp->getRawDamage();
		auto oldOnCollisionAction = emp->getOnCollisionAction();
		auto oldPierceResetTime = emp->getRawPierceResetTime();
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
			emp->setHitboxRadius(newEMP->getRawHitboxRadius());
			emp->setDespawnTime(newEMP->getDespawnTime());
			emp->setSpawnType(newEMP->getSpawnType());
			emp->setShadowTrailLifespan(newEMP->getRawShadowTrailLifespan());
			emp->setShadowTrailInterval(newEMP->getRawShadowTrailInterval());
			emp->setDamage(newEMP->getRawDamage());
			emp->setOnCollisionAction(newEMP->getOnCollisionAction());
			emp->setPierceResetTime(newEMP->getRawPierceResetTime());
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
