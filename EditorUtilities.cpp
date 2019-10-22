#include "EditorUtilities.h"
#include "Constants.h"
#include <map>
#include <boost/filesystem.hpp>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#include <WinUser.h>
#endif

std::string getID(std::shared_ptr<EMPAAngleOffset> offset) {
	if (dynamic_cast<EMPAAngleOffsetZero*>(offset.get())) {
		return "0";
	} else if (dynamic_cast<EMPAAngleOffsetToPlayer*>(offset.get())) {
		return "1";
	} else if (dynamic_cast<EMPAAngleOffsetToGlobalPosition*>(offset.get())) {
		return "3";
	} else if (dynamic_cast<EMPAngleOffsetPlayerSpriteAngle*>(offset.get())) {
		return "4";
	}
}

void sendToForeground(sf::RenderWindow& window) {
	// Windows only
#ifdef _WIN32
	SetForegroundWindow(window.getSystemHandle());
#endif
	//TODO other OS's if necessary
}

std::shared_ptr<tgui::Label> createToolTip(std::string text) {
	auto tooltip = tgui::Label::create();
	tooltip->setMaximumTextWidth(300);
	tooltip->setText(text);
	tooltip->setTextSize(TEXT_SIZE);
	tooltip->getRenderer()->setBackgroundColor(tgui::Color::White);

	return tooltip;
}

sf::VertexArray generateVertexArray(std::vector<std::shared_ptr<EMPAction>> actions, float timeResolution, float x, float y, float playerX, float playerY, sf::Color startColor, sf::Color endColor) {
	float totalTime = 0;
	for (auto action : actions) {
		totalTime += action->getTime();
	}

	sf::VertexArray ret;
	float totalTimeElapsed = 0;
	float time = 0;
	float curX = x, curY = y;
	for (auto action : actions) {
		auto mp = action->generateStandaloneMP(curX, curY, playerX, playerY); 
		while (time < action->getTime()) {
			sf::Vector2f pos = mp->compute(sf::Vector2f(0, 0), time);
			sf::Color color = sf::Color((endColor.r - startColor.r)*(totalTimeElapsed / totalTime) + startColor.r, (endColor.r - startColor.r)*(totalTimeElapsed / totalTime) + startColor.r, (endColor.b - startColor.b)*(totalTimeElapsed / totalTime) + startColor.b, (endColor.a - startColor.a)*(totalTimeElapsed / totalTime) + startColor.a);
			ret.append(sf::Vertex(pos + sf::Vector2f(curX, curY), color));
			time += timeResolution;
			totalTimeElapsed += timeResolution;
		}
		time -= action->getTime();
		sf::Vector2f newPos = mp->compute(sf::Vector2f(0, 0), mp->getLifespan());
		curX += newPos.x;
		curY += newPos.y;
	}
	return ret;
}

sf::VertexArray generateVertexArray(std::shared_ptr<EMPAction> action, float timeResolution, float x, float y, float playerX, float playerY, sf::Color startColor, sf::Color endColor) {
	float totalTime = action->getTime();
	sf::VertexArray ret;
	float totalTimeElapsed = 0;
	float time = 0;
	auto mp = action->generateStandaloneMP(x, y, playerX, playerY);
	while (time < action->getTime()) {
		sf::Vector2f pos = mp->compute(sf::Vector2f(0, 0), time);
		sf::Color color = sf::Color((endColor.r - startColor.r)*(totalTimeElapsed / totalTime) + startColor.r, (endColor.r - startColor.r)*(totalTimeElapsed / totalTime) + startColor.r, (endColor.b - startColor.b)*(totalTimeElapsed / totalTime) + startColor.b, (endColor.a - startColor.a)*(totalTimeElapsed / totalTime) + startColor.a);
		ret.append(sf::Vertex(pos + sf::Vector2f(x, y), color));
		time += timeResolution;
		totalTimeElapsed += timeResolution;
	}
	time -= action->getTime();
	sf::Vector2f newPos = mp->compute(sf::Vector2f(0, 0), mp->getLifespan());
	return ret;
}

AnimatableChooser::AnimatableChooser(SpriteLoader& spriteLoader, bool forceSprite, float paddingX, float paddingY) : spriteLoader(spriteLoader), forceSprite(forceSprite), paddingX(paddingX), paddingY(paddingY) {
	onValueSet = std::make_shared<entt::SigH<void(Animatable)>>();
	animatable = tgui::ComboBox::create();
	rotationType = tgui::ComboBox::create();

	animatable->setChangeItemOnScroll(false);
	rotationType->setChangeItemOnScroll(false);

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
	animatable->connect("MouseEntered", [&]() {
		calculateItemsToDisplay();
	});
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
	rotationType->connect("MouseEntered", [&]() {
		calculateItemsToDisplay();
	});
	rotationType->connect("ItemSelected", [&](std::string item, std::string id) {
		if (animatable->getSelectedItem() != "" && !(rotationType->getSelectedItem() == item && rotationType->getSelectedItemId() == id)) {
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

void AnimatableChooser::setSelectedItem(Animatable animatable) {
	if (animatable.getAnimatableName() == "") {
		this->animatable->deselectItem();
	} else if (animatable.getSpriteSheetName() + "\\" + animatable.getAnimatableName() != this->animatable->getSelectedItemId()) {
		this->animatable->setSelectedItemById(animatable.getSpriteSheetName() + "\\" + animatable.getAnimatableName());
	}
}

void AnimatableChooser::onContainerResize(int containerWidth, int containerHeight) {
	animatable->setSize(containerWidth - paddingX * 2, TEXT_BOX_HEIGHT);
	rotationType->setSize(containerWidth - paddingX * 2, TEXT_BOX_HEIGHT);

	animatable->setPosition(paddingX, paddingY);
	rotationType->setPosition(tgui::bindLeft(animatable), tgui::bindBottom(animatable) + paddingY);

	setSize(containerWidth - paddingX, rotationType->getPosition().y + rotationType->getSize().y + paddingY);
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

void AnimatableChooser::setVisible(bool visible) {
	tgui::Group::setVisible(visible);
	animatablePicture->setVisible(visible);
}

void AnimatableChooser::setEnabled(bool enabled) {
	tgui::Group::setEnabled(enabled);
	animatable->setEnabled(enabled);
	rotationType->setEnabled(enabled);
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

    tgui::Slider::setChangeValueOnScroll(false);

	tgui::Slider::connect("ValueChanged", [&](float value) {
		editBox->setValue(value);
		if (onValueSet) {
			onValueSet->publish(value);
		}
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
	Slider::setValue(value);
	editBox->setValue(value);
}

void SliderWithEditBox::setEnabled(bool enabled) {
	Slider::setEnabled(enabled);
	editBox->setEnabled(enabled);
}

inline std::shared_ptr<entt::SigH<void(float)>> SliderWithEditBox::getOnValueSet() {
	if (!onValueSet) {
		onValueSet = std::make_shared<entt::SigH<void(float)>>();
	}
	return onValueSet;
}

void SliderWithEditBox::setVisible(bool visible) {
	Slider::setVisible(visible);
	editBox->setVisible(visible);
}

void SliderWithEditBox::onEditBoxValueSet(float value) {
	if (value > getMaximum() && !editBox->getHasMax()) {
		setMaximum(value);
	}
	if (value < getMinimum() && !editBox->getHasMin()) {
		setMaximum(value);
	}
	Slider::setValue(value);
}

SoundSettingsGroup::SoundSettingsGroup(std::string pathToSoundsFolder, float paddingX, float paddingY) : paddingX(paddingX), paddingY(paddingY) {
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
	
	populateFileNames(pathToSoundsFolder);

	enableAudio->connect("Changed", [&]() {
		if (ignoreSignal) return;
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
		if (ignoreSignal) return;
		onNewSoundSettings->publish(SoundSettings(fileName->getSelectedItem(), volume->getValue(), pitch->getValue(), !enableAudio->isChecked()));
	});
	volume->getOnValueSet()->sink().connect<SoundSettingsGroup, &SoundSettingsGroup::onVolumeChange>(this);
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
	ignoreSignal = true;
	enableAudio->setChecked(!init.isDisabled());
	fileName->setSelectedItem(init.getFileName());
	volume->setValue(init.getVolume());
	pitch->setValue(init.getPitch());
	ignoreSignal = false;
}

void SoundSettingsGroup::populateFileNames(std::string pathToSoundsFolder) {
	fileName->deselectItem();
	fileName->removeAllItems();

	// Populate fileName with all supported sound files in the directory
	for (const auto & entry : boost::filesystem::directory_iterator(pathToSoundsFolder)) {
		std::string filePath = entry.path().string();
		std::string type = filePath.substr(filePath.find_last_of('.'));
		if (!(type == ".wav" || type == ".ogg" || type == ".flac")) {
			continue;
		}
		std::string name = filePath.substr(filePath.find_last_of('\\') + 1);
		fileName->addItem(name);
	}
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

	setSize(containerWidth - paddingX, pitch->getPosition().y + pitch->getSize().y + paddingY);
}

std::shared_ptr<entt::SigH<void(SoundSettings)>> SoundSettingsGroup::getOnNewSoundSettingsSignal() {
	return onNewSoundSettings;
}

void SoundSettingsGroup::setEnabled(bool enabled) {
	enableAudio->setEnabled(enabled);
	fileName->setEnabled(enabled);
	volume->setEnabled(enabled);
	pitch->setEnabled(enabled);
}

void SoundSettingsGroup::onVolumeChange(float volume) {
	if (ignoreSignal) return;
	onNewSoundSettings->publish(SoundSettings(fileName->getSelectedItem(), volume, pitch->getValue(), !enableAudio->isChecked()));
}

void SoundSettingsGroup::onPitchChange(float pitch) {
	if (ignoreSignal) return;
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
		setText(formatNum(value));
		onValueSet->publish(value);
	});
	updateInputValidator();
}

float NumericalEditBoxWithLimits::getValue() {
	if (getText() == "") return 0;
	return std::stof(std::string(getText()));
}

void NumericalEditBoxWithLimits::setValue(int value) {
	setText(formatNum(value));
}

void NumericalEditBoxWithLimits::setValue(float value) {
	if (mustBeInt) {
		setText(formatNum(std::round(value)));
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
		setInputValidator("^-?[0-9]*$");
	} else {
		setInputValidator("^[0-9].*$");
	}
}

TFVGroup::TFVGroup(UndoStack& undoStack, float paddingX, float paddingY) : undoStack(undoStack), paddingX(paddingX), paddingY(paddingY) {
	onAttackTFVChange = std::make_shared<entt::SigH<void(std::shared_ptr<EMPAction>, std::shared_ptr<EditorMovablePoint>, std::shared_ptr<EditorAttack>)>>();
	onEMPATFVChange = std::make_shared<entt::SigH<void(std::shared_ptr<EMPAction>)>>();

	//TODO
	/*
	test = tgui::Slider::create();
	test->connect("ValueChanged", [&](float value) {
		if (ignoreSignal) return;
		undoStack.execute(UndoableCommand(
			[this, value]() {
			onTFVChange();
		},
		[this]() {
			onTFVChange();
		}));
	});
	add(test);
	*/
}

void TFVGroup::onContainerResize(int containerWidth, int containerHeight) {
	//TODO
	//test->setPosition(0, 0);
	//test->setSize(containerWidth, 20);

	setSize(containerWidth - paddingX, 25);
}

void TFVGroup::setTFV(std::shared_ptr<TFV> tfv, std::shared_ptr<EMPAction> parentEMPA, std::shared_ptr<EditorMovablePoint> parentEMP, std::shared_ptr<EditorAttack> parentAttack) {
	this->tfv = tfv;
	this->parentEMPA = parentEMPA;
	this->parentEMP = parentEMP;
	this->parentAttack = parentAttack;

	ignoreSignal = true;
	//TODO: set widget values
	//test->setValue(2);
	ignoreSignal = false;
}

void TFVGroup::setTFV(std::shared_ptr<TFV> tfv, std::shared_ptr<EMPAction> parentEMPA) {
	this->tfv = tfv;
	this->parentEMPA = parentEMPA;
	this->parentEMP = nullptr;
	this->parentAttack = nullptr;

	ignoreSignal = true;
	//TODO: set widget values
	ignoreSignal = false;
}

void TFVGroup::onTFVChange() {
	if (onAttackTFVChange && parentAttack) {
		onAttackTFVChange->publish(parentEMPA, parentEMP, parentAttack);
	}
	if (onEMPATFVChange) {
		onEMPATFVChange->publish(parentEMPA);
	}
}

EMPAAngleOffsetGroup::EMPAAngleOffsetGroup(UndoStack& undoStack) : undoStack(undoStack) {
	onAngleOffsetChange = std::make_shared<entt::SigH<void(std::shared_ptr<EMPAction>, std::shared_ptr<EditorMovablePoint>, std::shared_ptr<EditorAttack>)>>();

	offsetType = tgui::ComboBox::create();
	offsetType->setTextSize(TEXT_SIZE);
	// ID are from getID()
	offsetType->addItem("No offset", "1");
	offsetType->addItem("To player", "2");
	offsetType->addItem("To global position", "3");
	offsetType->addItem("Player sprite angle", "4");
	offsetType->connect("ItemSelected", [&](std::string item, std::string id) {
		//TODO
		if (ignoreSignal) return;
		undoStack.execute(UndoableCommand(
			[this]() {
			
		},
			[this]() {
			
		}));
	});
}

void EMPAAngleOffsetGroup::onContainerResize(int containerWidth, int containerHeight) {
	//TODO
}

ScrollableListBox::ScrollableListBox() {
	listBox = tgui::ListBox::create();
	add(listBox);
	listBox->setSize("100%", "100%");

	textWidthChecker = tgui::Label::create();
	textWidthChecker->setMaximumTextWidth(0);
}

void ScrollableListBox::onListBoxItemsChange() {
	float largestWidth = 0;
	for (auto str : listBox->getItems()) {
		textWidthChecker->setText(str);
		largestWidth = std::max(largestWidth, textWidthChecker->getSize().x);
	}
	listBox->setSize(std::max(getSize().x, largestWidth), "100%");
}

bool ScrollableListBox::mouseWheelScrolled(float delta, tgui::Vector2f pos) {
	// This override makes it so if mouse wheel is scrolled when the scrollbar can't be scrolled any more,
	// the event is not consumed so that this widget's parent can handle the event.

	if (delta < 0) {
		// Super ghetto way of checking if scroll is already at max
		auto oldScrollbarValue = listBox->getScrollbarValue();
		listBox->setScrollbarValue(2000000000);
		if (listBox->getScrollbarValue() == oldScrollbarValue) {
			listBox->setScrollbarValue(oldScrollbarValue);
			return false;
		} else {
			listBox->setScrollbarValue(oldScrollbarValue);
			return tgui::ScrollablePanel::mouseWheelScrolled(delta, pos);
		}
	} else if (listBox->getScrollbarValue() == 0) {
		return false;
	} else {
		return tgui::ScrollablePanel::mouseWheelScrolled(delta, pos);
	}
}

void ScrollableListBox::setTextSize(int textSize) {
	listBox->setTextSize(textSize);
	textWidthChecker->setTextSize(textSize);
}

void Slider::setValue(float value) {
	// Round to nearest allowed value
	//if (m_step != 0)
	//	value = m_minimum + (std::round((value - m_minimum) / m_step) * m_step);

	// When the value is below the minimum or above the maximum then adjust it
	if (value < m_minimum)
		value = m_minimum;
	else if (value > m_maximum)
		value = m_maximum;

	if (m_value != value) {
		m_value = value;

		onValueChange.emit(this, m_value);

		updateThumbPosition();
	}
}

void Slider::setStep(float step) {
	m_step = step;
}