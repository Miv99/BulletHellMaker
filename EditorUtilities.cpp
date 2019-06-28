#include "EditorUtilities.h"
#include "Constants.h"
#include <map>

std::shared_ptr<tgui::Label> createToolTip(std::string text) {
	auto tooltip = tgui::Label::create();
	tooltip->setMaximumTextWidth(300);
	tooltip->setText(text);
	tooltip->setTextSize(TEXT_SIZE);
	tooltip->getRenderer()->setBackgroundColor(tgui::Color::White);

	return tooltip;
}

AnimatableChooser::AnimatableChooser(SpriteLoader& spriteLoader, bool forceSprite, float paddingX, float paddingY) : spriteLoader(spriteLoader), forceSprite(forceSprite), paddingX(paddingX), paddingY(paddingY) {
	onValueSet = std::make_shared<entt::SigH<void(Animatable)>>();
	animatable = tgui::ComboBox::create();
	rotationType = tgui::ComboBox::create();

	const std::map<std::string, std::shared_ptr<SpriteSheet>> sheets = spriteLoader.getSpriteSheets();
	for (auto it = sheets.begin(); it != sheets.end(); it++) {
		const std::map<std::string, std::shared_ptr<SpriteData>> spriteData = it->second->getSpriteData();
		animatable->addItem(it->first);
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
	rotationType->addItem("Never roate", std::to_string(static_cast<int>(LOCK_ROTATION)));
	rotationType->addItem("Face horizontal movement", std::to_string(static_cast<int>(LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT)));

	animatable->setChangeItemOnScroll(false);
	animatable->setExpandDirection(tgui::ComboBox::ExpandDirection::Down);
	animatable->connect("ItemSelected", [&](std::string itemText, std::string id) {
		// ID is in format "spriteSheetName\animatableName"
		std::string spriteSheetName = id.substr(0, id.find_first_of('\\'));

		// The only items without an ID are sprite sheet name indicators, so if ID is empty, this item shouldn't be selectable
		if (spriteSheetName == "") {
			animatable->deselectItem();
		} else {
			// Item text is in format "[S]spriteName" or "[A]animationName"
			onValueSet->publish(Animatable(itemText.substr(3), spriteSheetName, itemText[1] == 'S', static_cast<ROTATION_TYPE>(std::stoi(std::string(rotationType->getSelectedItemId())))));
		}
	});

	rotationType->setChangeItemOnScroll(false);
	rotationType->setExpandDirection(tgui::ComboBox::ExpandDirection::Down);
	rotationType->connect("ItemSelected", [&](std::string item, std::string id) {
		if (animatable->getSelectedItem() != "") {
			std::string spriteSheetName = std::string(animatable->getSelectedItemId()).substr(0, std::string(animatable->getSelectedItemId()).find_first_of('\\'));

			onValueSet->publish(Animatable(std::string(animatable->getSelectedItem()).substr(3), spriteSheetName, animatable->getSelectedItem()[1] == 'S', static_cast<ROTATION_TYPE>(std::stoi(std::string(id)))));
		}
	});
	rotationType->setSelectedItemById(std::to_string(static_cast<int>(LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT)));

	add(animatable);
	add(rotationType);
}

void AnimatableChooser::calculateItemsToDisplay() {
	if (!getParent()) {
		return;
	}

	if (animatable->getExpandDirection() == tgui::ComboBox::ExpandDirection::Down) {
		float availableSpace = getParent()->getSize().y - getPosition().y - getSize().y;
		// Each item in the ComboBox has height equal to the ComboBox's height
		animatable->setItemsToDisplay((int)(availableSpace / getSize().y));
	}
}

void AnimatableChooser::setSelectedItem(Animatable animatable) {
	if (animatable.getAnimatableName() == "") {
		this->animatable->deselectItem();
	} else {
		this->animatable->setSelectedItemById(animatable.getAnimatableName() + "\\" + animatable.getSpriteSheetName());
	}
}

void AnimatableChooser::onContainerResize(int containerWidth, int containerHeight) {
	animatable->setSize(containerWidth - paddingX * 2, TEXT_BOX_HEIGHT);
	rotationType->setSize(containerWidth - paddingX * 2, TEXT_BOX_HEIGHT);

	animatable->setPosition(paddingX, paddingY);
	rotationType->setPosition(tgui::bindLeft(animatable), tgui::bindBottom(animatable) + paddingY);

	setSize(getSize().x, rotationType->getPosition().y + rotationType->getSize().y + paddingY);
	calculateItemsToDisplay();
}

std::shared_ptr<AnimatablePicture> AnimatableChooser::getAnimatablePicture() {
	if (!animatablePicture) {
		animatablePicture = std::make_shared<AnimatablePicture>();
		animatable->connect("ItemSelected", [&](std::string itemText, std::string id) {
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
		});
	}
	return animatablePicture;
}

void AnimatableChooser::setVisible(bool visible) {
	tgui::Group::setVisible(visible);
	animatablePicture->setVisible(visible);
}

void AnimatablePicture::update(float deltaTime) {
	if (animation) {
		std::shared_ptr<sf::Sprite> sprite = animation->update(deltaTime);
		getRenderer()->setTexture(*sprite->getTexture());
	}
}

void AnimatablePicture::setAnimation(SpriteLoader& spriteLoader, const std::string& animationName, const std::string& spriteSheetName) {
	this->animation = spriteLoader.getAnimation(animationName, spriteSheetName, true);
}

void AnimatablePicture::setSprite(SpriteLoader& spriteLoader, const std::string& spriteName, const std::string& spriteSheetName) {
	getRenderer()->setTexture(*spriteLoader.getSprite(spriteName, spriteSheetName)->getTexture());
	animation = nullptr;
}

SliderWithEditBox::SliderWithEditBox(float paddingX) : paddingX(paddingX) {
	editBox = std::make_shared<NumericalEditBoxWithLimits>();

	tgui::Slider::connect("ValueChanged", [&](float value) {
		editBox->setValue(value);
	});
	editBox->getOnValueSet()->sink().connect<SliderWithEditBox, &SliderWithEditBox::onEditBoxValueSet>(this);
}

void SliderWithEditBox::setSize(float x, float y) {
	const float editBoxSize = TEXT_SIZE * 5;
	Slider::setSize(x - editBoxSize - paddingX * 2, y);
	editBox->setSize(editBoxSize, editBox->getSize().y);
}

void SliderWithEditBox::setPosition(float x, float y) {
	Slider::setPosition(x, y);
	editBox->setPosition(getPosition().x + getSize().x + paddingX, getPosition().y + (getSize().y - editBox->getSize().y)/2.0f);
}

void SliderWithEditBox::setValue(float value) {
	tgui::Slider::setValue(value);
	editBox->setValue(value);
}

void SliderWithEditBox::setEnabled(bool enabled) {
	tgui::Slider::setEnabled(enabled);
	editBox->setEnabled(enabled);
}

inline std::shared_ptr<entt::SigH<void(float)>> SliderWithEditBox::getOnValueSet() {
	if (!onValueSet) {
		onValueSet = std::make_shared<entt::SigH<void(float)>>();
	}
	return onValueSet;
}

void SliderWithEditBox::setVisible(bool visible) {
	tgui::Slider::setVisible(visible);
	editBox->setVisible(visible);
}

void SliderWithEditBox::onEditBoxValueSet(float value) {
	if (value > getMaximum() && !editBox->getHasMax()) {
		setMaximum(value);
	}
	if (value < getMinimum() && !editBox->getHasMin()) {
		setMaximum(value);
	}
	tgui::Slider::setValue(value);
}

SoundSettingsGroup::SoundSettingsGroup(float paddingX, float paddingY) : paddingX(paddingX), paddingY(paddingY) {
	onNewSoundSettings = std::make_shared<entt::SigH<void(SoundSettings)>>();
	enableAudio = tgui::CheckBox::create();
	fileName = tgui::ComboBox::create();
	volume = std::make_shared<SliderWithEditBox>();
	pitch = std::make_shared<SliderWithEditBox>();
	enableAudioLabel = tgui::Label::create();
	fileNameLabel = tgui::Label::create();
	volumeLabel = tgui::Label::create();
	pitchLabel = tgui::Label::create();

	volume->setMinimum(0);
	volume->setMaximum(1.0f);
	volume->setStep(0.01f);
	volume->getEditBox()->setMaximumCharacters(4);
	pitch->setMinimum(1);
	pitch->setMaximum(10);
	pitch->setStep(0.01f);
	pitch->getEditBox()->setMaximumCharacters(4);

	enableAudioLabel->setTextSize(TEXT_SIZE);
	fileNameLabel->setTextSize(TEXT_SIZE);
	volumeLabel->setTextSize(TEXT_SIZE);
	pitchLabel->setTextSize(TEXT_SIZE);

	enableAudioLabel->setText("Enable sound on spawn");
	fileNameLabel->setText("Sound file");
	volumeLabel->setText("Volume");
	pitchLabel->setText("Pitch");
	
	//TODO:pass in sound folder and auotmatically populate fileName

	enableAudio->connect("Changed", [&]() {
		onNewSoundSettings->publish(SoundSettings(fileName->getSelectedItem(), volume->getValue(), pitch->getValue(), !enableAudio->isChecked()));
		if (enableAudio->isChecked()) {
			fileName->setEnabled(true);
			volume->setEnabled(true);
			pitch->setEnabled(true);
		} else {
			fileName->setEnabled(false);
			volume->setEnabled(false);
			pitch->setEnabled(false);
		}
	});
	fileName->connect("ItemSelected", [&]() {
		onNewSoundSettings->publish(SoundSettings(fileName->getSelectedItem(), volume->getValue(), pitch->getValue(), !enableAudio->isChecked()));
	});
	pitch->getOnValueSet()->sink().connect<SoundSettingsGroup, &SoundSettingsGroup::onVolumeChange>(this);
	pitch->getOnValueSet()->sink().connect<SoundSettingsGroup, &SoundSettingsGroup::onPitchChange>(this);

	add(enableAudio);
	add(fileName);
	add(volume);
	add(volume->getEditBox());
	add(pitch);
	add(pitch->getEditBox());
	add(enableAudioLabel);
	add(fileNameLabel);
	add(volumeLabel);
	add(pitchLabel);
}

void SoundSettingsGroup::initSettings(SoundSettings init) {
	enableAudio->setChecked(!init.isDisabled());
	fileName->setSelectedItem(init.getFileName());
	volume->setValue(init.getVolume());
	pitch->setValue(init.getPitch());
}

void SoundSettingsGroup::onContainerResize(int containerWidth, int containerHeight) {
	enableAudio->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	fileName->setSize(containerWidth - paddingX * 2, TEXT_BOX_HEIGHT);
	volume->setSize(containerWidth - paddingX * 2, SLIDER_HEIGHT);
	pitch->setSize(containerWidth - paddingX * 2, SLIDER_HEIGHT);
	enableAudioLabel->setSize(containerWidth - paddingX * 2, TEXT_BOX_HEIGHT);
	fileNameLabel->setSize(containerWidth - paddingX * 2, TEXT_BOX_HEIGHT);
	volumeLabel->setSize(containerWidth - paddingX * 2, TEXT_BOX_HEIGHT);
	pitchLabel->setSize(containerWidth - paddingX * 2, TEXT_BOX_HEIGHT);

	enableAudioLabel->setPosition(paddingX, paddingY);
	enableAudio->setPosition(tgui::bindLeft(enableAudioLabel), tgui::bindBottom(enableAudioLabel) + paddingY);
	fileNameLabel->setPosition(tgui::bindLeft(enableAudioLabel), tgui::bindBottom(enableAudio) + paddingY);
	fileName->setPosition(tgui::bindLeft(enableAudioLabel), tgui::bindBottom(fileNameLabel) + paddingY);
	volumeLabel->setPosition(tgui::bindLeft(enableAudioLabel), tgui::bindBottom(fileName) + paddingY);
	volume->setPosition(enableAudioLabel->getPosition().x, volumeLabel->getPosition().y + volumeLabel->getSize().y + paddingY);
	pitchLabel->setPosition(tgui::bindLeft(enableAudioLabel), tgui::bindBottom(volume) + paddingY);
	pitch->setPosition(enableAudioLabel->getPosition().x, pitchLabel->getPosition().y + pitchLabel->getSize().y + paddingY);

	setSize(getSize().x, pitch->getPosition().y + pitch->getSize().y + paddingY);
}

std::shared_ptr<entt::SigH<void(SoundSettings)>> SoundSettingsGroup::getOnNewSoundSettingsSignal() {
	return onNewSoundSettings;
}

void SoundSettingsGroup::onVolumeChange(float volume) {
	onNewSoundSettings->publish(SoundSettings(fileName->getSelectedItem(), volume, pitch->getValue(), !enableAudio->isChecked()));
}

void SoundSettingsGroup::onPitchChange(float pitch) {
	onNewSoundSettings->publish(SoundSettings(fileName->getSelectedItem(), volume->getValue(), pitch, !enableAudio->isChecked()));
}

NumericalEditBoxWithLimits::NumericalEditBoxWithLimits() {
	connect("ReturnKeyPressed", [&](std::string text) {
		float value = stof(text);
		if (hasMax && value > max) {
			value = max;
		}
		if (hasMin && value < min) {
			value = min;
		}
		setText(std::to_string(value));
		onValueSet->publish(value);
	});
	updateInputValidator();
}

float NumericalEditBoxWithLimits::getValue() {
	if (getText() == "") return 0;
	return std::stof(std::string(getText()));
}

void NumericalEditBoxWithLimits::setValue(int value) {
	setText(std::to_string(value));
}

void NumericalEditBoxWithLimits::setValue(float value) {
	if (mustBeInt) {
		setValue(std::round(value));
	} else {
		setText(formatNum(value));
	}
}

void NumericalEditBoxWithLimits::setMin(float min) {
	this->min = min;
	hasMin = true;
}

void NumericalEditBoxWithLimits::setMax(float max) {
	this->max = max;
	hasMax = true;
}

void NumericalEditBoxWithLimits::removeMin() {
	hasMin = false;
}

void NumericalEditBoxWithLimits::removeMax() {
	hasMax = false;
}

void NumericalEditBoxWithLimits::setIntegerMode(bool intMode) {
	mustBeInt = intMode;
	updateInputValidator();
}

std::shared_ptr<entt::SigH<void(float)>> NumericalEditBoxWithLimits::getOnValueSet() {
	if (!onValueSet) {
		onValueSet = std::make_shared<entt::SigH<void(float)>>();
	}
	return onValueSet;
}

void NumericalEditBoxWithLimits::updateInputValidator() {
	if (mustBeInt) {
		setInputValidator("^[0-9]*$");
	} else {
		setInputValidator("^[0-9].*$");
	}
}

TFVGroup::TFVGroup() {
	onTFVChange = std::make_shared<entt::SigH<void()>>();
}

void TFVGroup::onContainerResize(int containerWidth, int containerHeight) {
	//TODO
}

EMPAAngleOffsetGroup::EMPAAngleOffsetGroup() {
	onAngleOffsetChange = std::make_shared <entt::SigH<void()>>();
}

void EMPAAngleOffsetGroup::onContainerResize(int containerWidth, int containerHeight) {
	//TODO
}
