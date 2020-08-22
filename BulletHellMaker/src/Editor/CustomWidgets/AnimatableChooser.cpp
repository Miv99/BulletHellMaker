#include <Editor/CustomWidgets/AnimatableChooser.h>

#include <GuiConfig.h>
#include <Editor/Util/EditorUtils.h>

AnimatableChooser::AnimatableChooser(SpriteLoader& spriteLoader, bool forceSprite) : spriteLoader(spriteLoader), forceSprite(forceSprite) {
	animatablePicture = AnimatablePicture::create();
	animatable = tgui::ComboBox::create();
	rotationType = tgui::ComboBox::create();

	animatablePicture->setPosition(0, 0);
	animatable->setPosition(tgui::bindLeft(animatablePicture), tgui::bindBottom(animatablePicture) + GUI_PADDING_Y);
	rotationType->setPosition(tgui::bindLeft(animatable), tgui::bindBottom(animatable) + GUI_PADDING_Y);

	animatablePicture->setSize(300, 300);
	animatable->setSize("100%", TEXT_BOX_HEIGHT);
	rotationType->setSize("100%", TEXT_BOX_HEIGHT);

	animatable->setChangeItemOnScroll(false);
	rotationType->setChangeItemOnScroll(false);

	const std::map<std::string, std::shared_ptr<SpriteSheet>> sheets = spriteLoader.getSpriteSheets();
	for (auto it = sheets.begin(); it != sheets.end(); it++) {
		const std::map<std::string, std::shared_ptr<SpriteData>> spriteData = it->second->getSpriteData();
		animatable->addItem(it->first, "Sheet");
		for (auto it2 = spriteData.begin(); it2 != spriteData.end(); it2++) {
			animatable->addItem("[S]" + it2->first, it->first + "\\" + it2->first);
		}

		if (!forceSprite) {
			const std::map<std::string, std::shared_ptr<AnimationData>> animationData = it->second->getAnimationData();
			for (auto it2 = animationData.begin(); it2 != animationData.end(); it2++) {
				animatable->addItem("[A]" + it2->first, it->first + "\\" + it2->first);
			}
		}
	}

	rotationType->addItem("Rotate with movement", std::to_string(static_cast<int>(ROTATE_WITH_MOVEMENT)));
	rotationType->addItem("Never rotate", std::to_string(static_cast<int>(LOCK_ROTATION)));
	rotationType->addItem("Face horizontal movement", std::to_string(static_cast<int>(LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT)));

	if (forceSprite) {
		animatable->setToolTip(createToolTip("The selected sprite."));
	} else {
		animatable->setToolTip(createToolTip("The selected sprite or animation. Sprites are denoted with [S] and animations with [A]."));
	}
	rotationType->setToolTip(createToolTip("The behavior of an entity with this sprite/animation in terms of rotation.\n\
\tRotate with movement - The entity's angle of rotation will be the angle of its current direction of movement.\n\
\tNever rotate - The entity's angle of rotation will always be 0.\n\
\tFace horizontal movement - The entity's angle of rotation will be either 0 (when it is moving at an angle in range (0, 90) \
or (270, 360)) or 180 (when it is moving at angle in range (90, 270))."));

	animatable->connect("ItemSelected", [&](std::string itemText, std::string id) {
		// ID is in format "spriteSheetName\animatableName"
		std::string spriteSheetName = id.substr(0, id.find_first_of('\\'));

		// If id is "Sheet", a sheet was selected, so reselect the previous selection
		if (id == "Sheet" && !ignoreSignals) {
			ignoreSignals = true;
			animatable->setSelectedItemById(previousAnimatableSelection);
			ignoreSignals = false;
			return;
		}

		// The only items without an ID are sprite sheet name indicators, so if ID is empty, this item shouldn't be selectable
		if (spriteSheetName == "" && !ignoreSignals) {
			animatable->deselectItem();
		} else if (!ignoreSignals) {
			previousAnimatableSelection = id;
			// Item text is in format "[S]spriteName" or "[A]animationName"
			if (itemText[1] == 'S') {
				animatablePicture->setSprite(spriteLoader, itemText.substr(3), spriteSheetName);
			} else {
				animatablePicture->setAnimation(spriteLoader, itemText.substr(3), spriteSheetName);
			}

			onValueChange.emit(this, Animatable(itemText.substr(3), spriteSheetName, itemText[1] == 'S', static_cast<ROTATION_TYPE>(std::stoi(std::string(rotationType->getSelectedItemId())))));
		} else {
			// Item text is in format "[S]spriteName" or "[A]animationName"
			if (itemText[1] == 'S') {
				animatablePicture->setSprite(spriteLoader, itemText.substr(3), spriteSheetName);
			} else {
				animatablePicture->setAnimation(spriteLoader, itemText.substr(3), spriteSheetName);
			}
		}

	});

	animatable->setChangeItemOnScroll(false);
	animatable->setExpandDirection(tgui::ComboBox::ExpandDirection::Down);
	animatable->connect("Focused", [&]() {
		calculateItemsToDisplay();
	});

	rotationType->setChangeItemOnScroll(false);
	rotationType->setExpandDirection(tgui::ComboBox::ExpandDirection::Down);
	rotationType->connect("Focused", [&]() {
		calculateItemsToDisplay();
	});
	rotationType->connect("ItemSelected", [&](std::string item, std::string id) {
		if (ignoreSignals) {
			return;
		}

		if (animatable->getSelectedItem() == "") {
			onValueChange.emit(this, Animatable("", "", false, static_cast<ROTATION_TYPE>(std::stoi(std::string(id)))));
		} else {
			std::string spriteSheetName = std::string(animatable->getSelectedItemId()).substr(0, std::string(animatable->getSelectedItemId()).find_first_of('\\'));

			onValueChange.emit(this, Animatable(std::string(animatable->getSelectedItem()).substr(3), spriteSheetName, animatable->getSelectedItem()[1] == 'S', static_cast<ROTATION_TYPE>(std::stoi(std::string(id)))));
		}
	});
	ignoreSignals = true;
	rotationType->setSelectedItemById(std::to_string(static_cast<int>(LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT)));
	ignoreSignals = false;

	animatable->getSharedRenderer()->setOpacityDisabled(WIDGET_OPACITY_DISABLED);
	rotationType->getSharedRenderer()->setOpacityDisabled(WIDGET_OPACITY_DISABLED);

	add(animatable);
	add(rotationType);
	add(animatablePicture);

	animatablePicture->connect("SizeChanged", [this](sf::Vector2f newSize) {
		this->setSize(this->getSizeLayout().x, tgui::bindBottom(this->rotationType));
	});
	connect("SizeChanged", [this](sf::Vector2f newSize) {
		calculateItemsToDisplay();
	});
}

void AnimatableChooser::calculateItemsToDisplay() {
	if (!getParent()) {
		return;
	}

	if (animatable->getExpandDirection() == tgui::ComboBox::ExpandDirection::Down) {
		auto window = getParent();
		while (window->getParent()) {
			window = window->getParent();
		}
		float heightLeft = window->getSize().y - animatable->getAbsolutePosition().y;
		// Each item in the ComboBox has height equal to the ComboBox's height
		animatable->setItemsToDisplay((int)(heightLeft / animatable->getSize().y));
	}
	if (rotationType->getExpandDirection() == tgui::ComboBox::ExpandDirection::Down) {
		auto window = getParent();
		while (window->getParent()) {
			window = window->getParent();
		}
		float heightLeft = window->getSize().y - rotationType->getAbsolutePosition().y;
		// Each item in the ComboBox has height equal to the ComboBox's height
		rotationType->setItemsToDisplay((int)(heightLeft / rotationType->getSize().y));
	}
}

void AnimatableChooser::setValue(Animatable animatable) {
	ignoreSignals = true;
	if (animatable.getAnimatableName() == "") {
		this->animatable->deselectItem();
	} else if (animatable.getSpriteSheetName() + "\\" + animatable.getAnimatableName() != this->animatable->getSelectedItemId()) {
		this->animatable->setSelectedItemById(animatable.getSpriteSheetName() + "\\" + animatable.getAnimatableName());
	}
	rotationType->setSelectedItemById(std::to_string(static_cast<int>(animatable.getRotationType())));
	ignoreSignals = false;
}

Animatable AnimatableChooser::getValue() {
	std::string itemText = animatable->getSelectedItem();
	if (itemText == "") {
		return Animatable("", "", false, LOCK_ROTATION);
	}
	std::string id = animatable->getSelectedItemId();

	// ID is in format "spriteSheetName\animatableName"
	std::string spriteSheetName = id.substr(0, id.find_first_of('\\'));

	// The only items without an ID are sprite sheet name indicators, so if ID is empty, this item shouldn't be selectable
	if (spriteSheetName == "") {
		animatable->deselectItem();
	} else {
		// Item text is in format "[S]spriteName" or "[A]animationName"
		if (itemText[1] == 'S') {
			animatablePicture->setSprite(spriteLoader, itemText.substr(3), spriteSheetName);
		} else {
			animatablePicture->setAnimation(spriteLoader, itemText.substr(3), spriteSheetName);
		}
	}

	return Animatable(itemText.substr(3), spriteSheetName, itemText[1] == 'S', static_cast<ROTATION_TYPE>(std::stoi(std::string(rotationType->getSelectedItemId()))));
}

void AnimatableChooser::setEnabled(bool enabled) {
	HideableGroup::setEnabled(enabled);
	animatable->setEnabled(enabled);
	rotationType->setEnabled(enabled);
}

void AnimatableChooser::setAnimatablePictureSize(tgui::Layout width, tgui::Layout height) {
	animatablePicture->setSize(width, height);
}

void AnimatableChooser::setAnimatablePictureSize(const tgui::Layout2d& size) {
	animatablePicture->setSize(size);
}

tgui::Signal& AnimatableChooser::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName())) {
		return onValueChange;
	}
	return HideableGroup::getSignal(signalName);
}