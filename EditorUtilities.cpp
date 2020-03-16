#include "EditorUtilities.h"
#include "Constants.h"
#include "EditorWindow.h"
#include <map>
#include <boost/filesystem.hpp>
#include <iostream>
#include <algorithm>
#include "Level.h"
#include "matplotlibcpp.h"

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

std::shared_ptr<tgui::ListBox> createMenuPopup(std::vector<std::pair<std::string, std::function<void()>>> elements) {
	auto popup = tgui::ListBox::create();
	int i = 0;
	int longestStringLength = 0;
	for (std::pair<std::string, std::function<void()>> element : elements) {
		longestStringLength = std::max(longestStringLength, (int)element.first.length());
		popup->addItem(element.first, std::to_string(i));
		i++;
	}
	popup->setItemHeight(20);
	popup->setSize(std::max(150, (int)(longestStringLength * popup->getTextSize())), (popup->getItemHeight() + 1) * popup->getItemCount());
	popup->connect("MousePressed", [popup, elements](std::string item, std::string id) {
		elements[std::stoi(id)].second();
		popup->deselectItem();
	});
	return popup;
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
	return ret;
}

std::vector<std::pair<std::vector<float>, std::vector<float>>> generateMPLPoints(std::shared_ptr<PiecewiseTFV> tfv, float tfvLifespan, float timeResolution) {
	std::vector<std::pair<std::vector<float>, std::vector<float>>> ret;
	float time = 0;
	int prevSegmentIndex = -1;
	float lowestY = 2147483647;
	float highestY = -2147483647;
	std::vector<float> singleSegmentX;
	std::vector<float> singleSegmentY;
	while (time < tfvLifespan) {
		std::pair<float, int> valueAndSegmentIndex = tfv->piecewiseEvaluate(time);
		sf::Vector2f pos(time, valueAndSegmentIndex.first);

		if (valueAndSegmentIndex.second != prevSegmentIndex) {
			if (prevSegmentIndex != -1) {
				ret.push_back(std::make_pair(singleSegmentX, singleSegmentY));
				singleSegmentX.clear();
				singleSegmentY.clear();
			}

			prevSegmentIndex = valueAndSegmentIndex.second;
		}
		lowestY = std::min(lowestY, valueAndSegmentIndex.first);
		highestY = std::max(highestY, valueAndSegmentIndex.first);

		singleSegmentX.push_back(pos.x);
		singleSegmentY.push_back(pos.y);
		time += timeResolution;
	}
	ret.push_back(std::make_pair(singleSegmentX, singleSegmentY));
	return ret;
}


const tgui::Layout2d & HideableGroup::getSizeLayout() const {
	if (!isVisible()) {
		return savedSize;
	}
	return tgui::Group::getSizeLayout();
}

void HideableGroup::setSize(const tgui::Layout2d & size) {
	// Ignore resizes until widget is set to visible again
	if (!isVisible()) {
		savedSize = size;
	} else {
		tgui::Group::setSize(size);
	}
}

void HideableGroup::setSize(tgui::Layout width, tgui::Layout height) {
	// Ignore resizes until widget is set to visible again
	if (!isVisible()) {
		savedSize = { width, height };
	} else {
		tgui::Group::setSize(width, height);
	}
}

void HideableGroup::setVisible(bool visible) {
	bool wasVisible = isVisible();

	tgui::Group::setVisible(visible);
	// Set width and height to 0 if this widget becomes invisible
	if (!visible && wasVisible) {
		savedSize = tgui::Group::getSizeLayout();
		tgui::Group::setSize({ 0, 0 });
	} else if (visible) {
		tgui::Group::setSize(savedSize);
	}
}

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
	rotationType->addItem("Never rotate", std::to_string(static_cast<int>(LOCK_ROTATION)));
	rotationType->addItem("Face horizontal movement", std::to_string(static_cast<int>(LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT)));

	animatable->connect("ItemSelected", [&](std::string itemText, std::string id) {
		if (ignoreSignals) {
			return;
		}

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
			onValueChange.emit(this, Animatable(itemText.substr(3), spriteSheetName, itemText[1] == 'S', static_cast<ROTATION_TYPE>(std::stoi(std::string(rotationType->getSelectedItemId())))));
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
	if (animatable.getAnimatableName() == "") {
		this->animatable->deselectItem();
	} else if (animatable.getSpriteSheetName() + "\\" + animatable.getAnimatableName() != this->animatable->getSelectedItemId()) {
		this->animatable->setSelectedItemById(animatable.getSpriteSheetName() + "\\" + animatable.getAnimatableName());
	}
	rotationType->setSelectedItemById(std::to_string(static_cast<int>(animatable.getRotationType())));
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
	tgui::Group::setEnabled(enabled);
	animatable->setEnabled(enabled);
	rotationType->setEnabled(enabled);
}

void AnimatableChooser::setAnimatablePictureSize(tgui::Layout width, tgui::Layout height) {
	animatablePicture->setSize(width, height);
}

void AnimatableChooser::setAnimatablePictureSize(const tgui::Layout2d & size) {
	animatablePicture->setSize(size);
}

tgui::Signal & AnimatableChooser::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName())) {
		return onValueChange;
	}
	return Group::getSignal(signalName);
}

AnimatablePicture::AnimatablePicture() {
	connect("SizeChanged", [&]() {
		resizeCurSpriteToFitWidget();
	});
}

AnimatablePicture::AnimatablePicture(const AnimatablePicture & other) {
}

void AnimatablePicture::update(sf::Time elapsedTime) {
	if (animation) {
		std::shared_ptr<sf::Sprite> prevSprite = curSprite;
		curSprite = animation->update(elapsedTime.asSeconds());
		if (curSprite != prevSprite) {
			resizeCurSpriteToFitWidget();
		}
	}
}

void AnimatablePicture::draw(sf::RenderTarget & target, sf::RenderStates states) const {
	if (curSprite) {
		if (spriteScaledToFitHorizontal) {
			curSprite->setPosition(getPosition().x + getSize().x / 2, getPosition().y + curSprite->getOrigin().y * curSprite->getScale().y);
		} else {
			curSprite->setPosition(getPosition().x + curSprite->getOrigin().x * curSprite->getScale().x, getPosition().y + getSize().y / 2);
		}
		target.draw(*curSprite, states);
	}
}

tgui::Widget::Ptr AnimatablePicture::clone() const {
	return std::make_shared<AnimatablePicture>(*this);
}

bool AnimatablePicture::mouseOnWidget(tgui::Vector2f pos) const {
	if (tgui::FloatRect{ 0, 0, getSize().x, getSize().y }.contains(pos)) {
		return true;
	}
	return false;
}

void AnimatablePicture::setAnimation(SpriteLoader& spriteLoader, const std::string& animationName, const std::string& spriteSheetName) {
	this->animation = spriteLoader.getAnimation(animationName, spriteSheetName, true);
}

void AnimatablePicture::setSprite(SpriteLoader& spriteLoader, const std::string& spriteName, const std::string& spriteSheetName) {
	curSprite = spriteLoader.getSprite(spriteName, spriteSheetName);
	animation = nullptr;
	resizeCurSpriteToFitWidget();
}

void AnimatablePicture::resizeCurSpriteToFitWidget() {
	if (!curSprite) {
		return;
	}
	
	float width = curSprite->getLocalBounds().width * curSprite->getScale().x;
	float height = curSprite->getLocalBounds().height * curSprite->getScale().y;
	float scaleAmount;
	if (width > height) {
		scaleAmount = getSize().x / width;
		spriteScaledToFitHorizontal = true;
	} else {
		scaleAmount = getSize().y / height;
		spriteScaledToFitHorizontal = false;
	}
	curSprite->scale(scaleAmount, scaleAmount);
}

SliderWithEditBox::SliderWithEditBox() {
	slider = std::make_shared<DelayedSlider>();
	editBox = std::make_shared<NumericalEditBoxWithLimits>();

	editBox->setPosition(tgui::bindRight(slider) + GUI_PADDING_X, 0);

    slider->setChangeValueOnScroll(false);
	slider->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) {
			return;
		}

		ignoreSignals = true;
		editBox->setValue(value);
		ignoreSignals = false;

		if (value != lastKnownValue) {
			onValueChange.emit(this, value);
			lastKnownValue = value;
		}
	});
	editBox->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) {
			return;
		}

		ignoreSignals = true;
		slider->setValue(value);
		ignoreSignals = false;

		if (value != lastKnownValue) {
			onValueChange.emit(this, value);
			lastKnownValue = value;
		}
	});

	connect("SizeChanged", [&](sf::Vector2f newSize) {
		editBox->setSize(tgui::bindMin(200, "30%"), "100%");
		// The sliding part of the slider extends upwards and downwards a bit, so make room for it
		slider->setSize(newSize.x - tgui::bindWidth(editBox) - GUI_PADDING_X - 4, newSize.y - 6);
		slider->setPosition(4, 3);
	});

	setMin(0);
	setMax(100);
	setIntegerMode(false);
	
	add(slider);
	add(editBox);

	lastKnownValue = slider->getValue();
}

float SliderWithEditBox::getValue() {
	return editBox->getValue();
}

void SliderWithEditBox::setValue(int value) {
	slider->setValue(value);
	editBox->setValue(value);
}

void SliderWithEditBox::setValue(float value) {
	slider->setValue(value);
	editBox->setValue(value);
}

void SliderWithEditBox::setMin(float min) {
	editBox->setMin(min);
	slider->setMinimum(min);
}

void SliderWithEditBox::setMax(float max) {
	editBox->setMax(max);
	slider->setMaximum(max);
}

void SliderWithEditBox::setStep(float step) {
	slider->setStep(step);
}

void SliderWithEditBox::setIntegerMode(bool intMode) {
	editBox->setIntegerMode(intMode);
	if (intMode) {
		slider->setStep(std::max((int)slider->getStep(), 1));
	}
}

void SliderWithEditBox::setTextSize(int textSize) {
	editBox->setTextSize(textSize);
}

tgui::Signal & SliderWithEditBox::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName())) {
		return onValueChange;
	}
	return Group::getSignal(signalName);
}

SoundSettingsGroup::SoundSettingsGroup(std::string pathToSoundsFolder) {
	enableAudio = tgui::CheckBox::create();
	fileName = tgui::ComboBox::create();
	volume = std::make_shared<SliderWithEditBox>();
	pitch = std::make_shared<SliderWithEditBox>();
	enableAudioLabel = tgui::Label::create();
	fileNameLabel = tgui::Label::create();
	volumeLabel = tgui::Label::create();
	pitchLabel = tgui::Label::create();

	volume->setMin(0);
	volume->setMax(1.0f);
	volume->setStep(0.01f);
	pitch->setMin(1);
	pitch->setMax(10);
	pitch->setStep(0.01f);

	enableAudioLabel->setTextSize(TEXT_SIZE);
	fileNameLabel->setTextSize(TEXT_SIZE);
	volumeLabel->setTextSize(TEXT_SIZE);
	pitchLabel->setTextSize(TEXT_SIZE);

	enableAudioLabel->setText("Enable sound on spawn");
	fileNameLabel->setText("Sound file");
	volumeLabel->setText("Volume");
	pitchLabel->setText("Pitch");

	enableAudio->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
	fileName->setSize("100%", TEXT_BOX_HEIGHT);
	volume->setSize("100%", SLIDER_HEIGHT);
	pitch->setSize("100%", SLIDER_HEIGHT);
	enableAudioLabel->setSize("100%", TEXT_BOX_HEIGHT);
	fileNameLabel->setSize("100%", TEXT_BOX_HEIGHT);
	volumeLabel->setSize("100%", TEXT_BOX_HEIGHT);
	pitchLabel->setSize("100%", TEXT_BOX_HEIGHT);

	enableAudioLabel->setPosition(0, 0);
	enableAudio->setPosition(tgui::bindLeft(enableAudioLabel), tgui::bindBottom(enableAudioLabel) + GUI_LABEL_PADDING_Y);
	fileNameLabel->setPosition(tgui::bindLeft(enableAudioLabel), tgui::bindBottom(enableAudio) + GUI_PADDING_Y);
	fileName->setPosition(tgui::bindLeft(enableAudioLabel), tgui::bindBottom(fileNameLabel) + GUI_LABEL_PADDING_Y);
	volumeLabel->setPosition(tgui::bindLeft(enableAudioLabel), tgui::bindBottom(fileName) + GUI_PADDING_Y);
	volume->setPosition(tgui::bindLeft(enableAudioLabel), tgui::bindBottom(volumeLabel) + GUI_LABEL_PADDING_Y);
	pitchLabel->setPosition(tgui::bindLeft(enableAudioLabel), tgui::bindBottom(volume) + GUI_PADDING_Y);
	pitch->setPosition(tgui::bindLeft(enableAudioLabel), tgui::bindBottom(pitchLabel) + GUI_LABEL_PADDING_Y);
	
	populateFileNames(pathToSoundsFolder);

	enableAudio->connect("Changed", [&]() {
		if (ignoreSignals) {
			return;
		}

		onValueChange.emit(this, SoundSettings(fileName->getSelectedItem(), volume->getValue(), pitch->getValue(), !enableAudio->isChecked()));
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
		if (ignoreSignals) {
			return;
		}

		onValueChange.emit(this, SoundSettings((std::string)fileName->getSelectedItem(), volume->getValue(), pitch->getValue(), !enableAudio->isChecked()));
	});
	volume->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) {
			return;
		}

		onValueChange.emit(this, SoundSettings((std::string)fileName->getSelectedItem(), volume->getValue(), pitch->getValue(), !enableAudio->isChecked()));
	});
	pitch->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) {
			return;
		}

		onValueChange.emit(this, SoundSettings((std::string)fileName->getSelectedItem(), volume->getValue(), pitch->getValue(), !enableAudio->isChecked()));
	});

	add(enableAudio);
	add(fileName);
	add(volume);
	add(pitch);
	add(enableAudioLabel);
	add(fileNameLabel);
	add(volumeLabel);
	add(pitchLabel);
}

void SoundSettingsGroup::initSettings(SoundSettings init) {
	ignoreSignals = true;
	enableAudio->setChecked(!init.isDisabled());
	fileName->setSelectedItem(init.getFileName());
	volume->setValue(init.getVolume());
	pitch->setValue(init.getPitch());
	ignoreSignals = false;
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

void SoundSettingsGroup::setEnabled(bool enabled) {
	enableAudio->setEnabled(enabled);
	fileName->setEnabled(enabled);
	volume->setEnabled(enabled);
	pitch->setEnabled(enabled);
}

tgui::Signal & SoundSettingsGroup::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName())) {
		return onValueChange;
	}
	return Group::getSignal(signalName);
}

NumericalEditBoxWithLimits::NumericalEditBoxWithLimits() {
	connect("ReturnKeyPressed", [&](std::string text) {
		if (ignoreSignals) {
			return;
		}

		try {
			float value = stof(text);
			if (hasMax && value > max) {
				value = max;
			}
			if (hasMin && value < min) {
				value = min;
			}

			ignoreSignals = true;
			setText(formatNum(value));
			ignoreSignals = false;

			onValueChange.emit(this, value);
		} catch (...) {

		}
	});
	connect("Unfocused", [&]() {
		if (ignoreSignals) {
			return;
		}

		try {
			float value = std::stof((std::string)getText());
			if (hasMax && value > max) {
				value = max;
			}
			if (hasMin && value < min) {
				value = min;
			}

			ignoreSignals = true;
			setText(formatNum(value));
			ignoreSignals = false;

			onValueChange.emit(this, value);
		} catch (...) {

		}
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
	if (getValue() < min) {
		setValue(min);
	}
	hasMin = true;
}

void NumericalEditBoxWithLimits::setMax(float max) {
	this->max = max;
	if (getValue() > max) {
		setValue(max);
	}
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

tgui::Signal & NumericalEditBoxWithLimits::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName())) {
		return onValueChange;
	}
	return EditBox::getSignal(signalName);
}

void NumericalEditBoxWithLimits::updateInputValidator() {
	if (mustBeInt) {
		setInputValidator("^-?[0-9]*$");
	} else {
		setInputValidator("^[0-9]*(\.[0-9]*)?$");
	}
}

TFVGroup::TFVGroup(EditorWindow& parentWindow) : parentWindow(parentWindow) {
	showGraph = tgui::Button::create();
	showGraph->setText("Show graph");
	showGraph->connect("Pressed", [&]() {
		auto points = generateMPLPoints(tfv, tfvLifespan, TFV_TIME_RESOLUTION);
		matplotlibcpp::clf();
		matplotlibcpp::close();
		for (int i = 0; i < points.size(); i++) {
			if (i % 2 == 0) {
				matplotlibcpp::plot(points[i].first, points[i].second, "r");
			} else {
				matplotlibcpp::plot(points[i].first, points[i].second, "b");
			}
		}
		matplotlibcpp::show();
	});
	add(showGraph);

	addSegment = tgui::Button::create();
	addSegment->setText("Add");
	addSegment->connect("Pressed", [&]() {
		if (ignoreSignals) return;

		std::lock_guard<std::recursive_mutex> lock(tfvMutex);

		float time = 0;
		if (selectedSegment) {
			time = tfv->getSegment(selectedSegmentIndex).first;
		}
		tfv->insertSegment(std::make_pair(time, std::make_shared<ConstantTFV>(0)), tfvLifespan);
		onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		if (selectedSegment) {
			selectSegment(selectedSegmentIndex + 1);
		}
		populateSegmentList();
	});
	add(addSegment);

	deleteSegment = tgui::Button::create();
	deleteSegment->setText("Delete");
	deleteSegment->connect("Pressed", [&]() {
		std::lock_guard<std::recursive_mutex> lock(tfvMutex);

		tfv->removeSegment(selectedSegmentIndex, tfvLifespan);
		populateSegmentList();
		if (tfv->getSegmentsCount() == 1) {
			selectSegment(0);
		} else if (selectedSegmentIndex < tfv->getSegmentsCount()) {
			selectSegment(selectedSegmentIndex);
		} else {
			selectSegment(selectedSegmentIndex - 1);
		}
	});
	add(deleteSegment);

	changeSegmentType = tgui::Button::create();
	changeSegmentType->setText("Change type");
	changeSegmentType->connect("Pressed", [&]() {
		parentWindow.addPopupWidget(segmentTypePopup, parentWindow.getMousePos().x, parentWindow.getMousePos().y, 200, segmentTypePopup->getSize().y);
	});
	add(changeSegmentType);

	segmentTypePopup = createMenuPopup({
		std::make_pair("Linear", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<LinearTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			// Reselect to reload widgets
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		}),
		std::make_pair("Constant", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<ConstantTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		}),
		std::make_pair("Sine wave", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<SineWaveTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		}),
		std::make_pair("Distance from acceleration", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<ConstantAccelerationDistanceTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		}),
		std::make_pair("Dampened start", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<DampenedStartTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		}),
		std::make_pair("Dampened end", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<DampenedEndTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		}),
		std::make_pair("Double dampened", [&]() {
			std::lock_guard<std::recursive_mutex> lock(tfvMutex);

			std::shared_ptr<TFV> newTFV = std::make_shared<DoubleDampenedTFV>();
			float oldStartTime = tfv->getSegment(selectedSegmentIndex).first;
			int selectedSegmentIndex = this->selectedSegmentIndex;
			tfv->changeSegment(selectedSegmentIndex, newTFV, tfvLifespan);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			selectSegment(selectedSegmentIndex);
			populateSegmentList();
		})
	});
	segmentTypePopup->setPosition(tgui::bindLeft(changeSegmentType), tgui::bindTop(changeSegmentType));

	segmentList = std::make_shared<ListBoxScrollablePanel>();
	segmentList->setTextSize(TEXT_SIZE);
	segmentList->getListBox()->connect("ItemSelected", [&](std::string item, std::string id) {
		if (ignoreSignals) return;
		if (id != "") {
			selectSegment(std::stoi(id));
		} else {
			deselectSegment();
		}
	});
	add(segmentList);

	tfvFloat1Label = tgui::Label::create();
	tfvFloat2Label = tgui::Label::create();
	tfvFloat3Label = tgui::Label::create();
	tfvFloat4Label = tgui::Label::create();
	tfvInt1Label = tgui::Label::create();
	startTimeLabel = tgui::Label::create();
	startTimeLabel->setText("Start time");
	startTimeLabel->setTextSize(TEXT_SIZE);
	tfvFloat1Label->setTextSize(TEXT_SIZE);
	tfvFloat2Label->setTextSize(TEXT_SIZE);
	tfvFloat3Label->setTextSize(TEXT_SIZE);
	tfvFloat4Label->setTextSize(TEXT_SIZE);
	tfvInt1Label->setTextSize(TEXT_SIZE);
	add(tfvFloat1Label);
	add(tfvFloat2Label);
	add(tfvFloat3Label);
	add(tfvFloat4Label);
	add(tfvInt1Label);
	add(startTimeLabel);

	tfvFloat1Slider = NumericalEditBoxWithLimits::create();
	tfvFloat2Slider = NumericalEditBoxWithLimits::create();
	tfvFloat3Slider = NumericalEditBoxWithLimits::create();
	tfvFloat4Slider = NumericalEditBoxWithLimits::create();
	tfvInt1Slider = NumericalEditBoxWithLimits::create();
	tfvInt1Slider->setIntegerMode(true);
	startTime = SliderWithEditBox::create();
	tfvFloat1Slider->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) return;

		if (dynamic_cast<LinearTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<LinearTFV*>(selectedSegment.get());
			ptr->setStartValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
			//TODO: update oldTFV after this?
		} else if (dynamic_cast<ConstantTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<ConstantTFV*>(selectedSegment.get());
			ptr->setValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<SineWaveTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<SineWaveTFV*>(selectedSegment.get());
			ptr->setPeriod(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get());
			ptr->setInitialDistance(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DampenedStartTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DampenedStartTFV*>(selectedSegment.get());
			ptr->setStartValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DampenedEndTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DampenedEndTFV*>(selectedSegment.get());
			ptr->setStartValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get());
			ptr->setStartValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		}
	});
	tfvFloat2Slider->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) return;

		if (dynamic_cast<LinearTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<LinearTFV*>(selectedSegment.get());
			ptr->setEndValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<SineWaveTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<SineWaveTFV*>(selectedSegment.get());
			ptr->setAmplitude(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get());
			ptr->setInitialVelocity(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DampenedStartTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DampenedStartTFV*>(selectedSegment.get());
			ptr->setEndValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DampenedEndTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DampenedEndTFV*>(selectedSegment.get());
			ptr->setEndValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get());
			ptr->setEndValue(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		}
	});
	tfvFloat3Slider->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) return;

		if (dynamic_cast<SineWaveTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<SineWaveTFV*>(selectedSegment.get());
			ptr->setValueShift(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get());
			ptr->setAcceleration(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		}
	});
	tfvFloat4Slider->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) return;

		if (dynamic_cast<SineWaveTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<SineWaveTFV*>(selectedSegment.get());
			ptr->setPhaseShift(value);
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		}
	});
	tfvInt1Slider->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) return;

		if (dynamic_cast<DampenedStartTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DampenedStartTFV*>(selectedSegment.get());
			ptr->setDampeningFactor(std::round(value));
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DampenedEndTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DampenedEndTFV*>(selectedSegment.get());
			ptr->setDampeningFactor(std::round(value));
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		} else if (dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get()) != nullptr) {
			auto ptr = dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get());
			ptr->setDampeningFactor(std::round(value));
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		}
	});
	startTime->connect("ValueChanged", [&, &selectedSegmentIndex = this->selectedSegmentIndex](float value) {
		if (ignoreSignals) return;

		std::lock_guard<std::recursive_mutex> lock(tfvMutex);
		if (selectedSegmentIndex != -1) {
			selectSegment(tfv->changeSegmentStartTime(selectedSegmentIndex, value, tfvLifespan));
			onValueChange.emit(this, std::make_pair(oldTFV, tfv));
		}
		populateSegmentList();
	});
	tfvFloat1Slider->setTextSize(TEXT_SIZE);
	tfvFloat2Slider->setTextSize(TEXT_SIZE);
	tfvFloat3Slider->setTextSize(TEXT_SIZE);
	tfvFloat4Slider->setTextSize(TEXT_SIZE);
	tfvInt1Slider->setTextSize(TEXT_SIZE);
	startTime->setTextSize(TEXT_SIZE);
	add(tfvFloat1Slider);
	add(tfvFloat2Slider);
	add(tfvFloat3Slider);
	add(tfvFloat4Slider);
	add(tfvInt1Slider);
	add(startTime);

	connect("SizeChanged", [&](sf::Vector2f newSize) {
		if (ignoreResizeSignal) {
			return;
		}

		ignoreResizeSignal = true;

		showGraph->setSize(100, TEXT_BUTTON_HEIGHT);
		showGraph->setPosition(0, 0);

		segmentList->setSize("40%", 250);
		segmentList->setPosition(tgui::bindLeft(showGraph), tgui::bindBottom(showGraph) + GUI_PADDING_Y);

		float buttonWidth = (segmentList->getSize().x - GUI_PADDING_X * 2) / 3.0f;

		addSegment->setSize(buttonWidth, TEXT_BUTTON_HEIGHT);
		addSegment->setPosition(tgui::bindLeft(segmentList), tgui::bindBottom(segmentList) + GUI_PADDING_Y);
		deleteSegment->setSize(buttonWidth, TEXT_BUTTON_HEIGHT);
		deleteSegment->setPosition(tgui::bindRight(addSegment) + GUI_PADDING_X, tgui::bindTop(addSegment));
		changeSegmentType->setSize(buttonWidth, TEXT_BUTTON_HEIGHT);
		changeSegmentType->setPosition(tgui::bindRight(deleteSegment) + GUI_PADDING_X, tgui::bindTop(deleteSegment));

		int segmentListRightBoundary = segmentList->getPosition().x + segmentList->getSize().x + GUI_PADDING_X;

		startTime->setSize(newSize.x - (segmentListRightBoundary + GUI_PADDING_X * 2), TEXT_BOX_HEIGHT);
		tfvFloat1Slider->setSize(newSize.x - (segmentListRightBoundary + GUI_PADDING_X * 2), TEXT_BOX_HEIGHT);
		tfvFloat2Slider->setSize(newSize.x - (segmentListRightBoundary + GUI_PADDING_X * 2), TEXT_BOX_HEIGHT);
		tfvFloat3Slider->setSize(newSize.x - (segmentListRightBoundary + GUI_PADDING_X * 2), TEXT_BOX_HEIGHT);
		tfvFloat4Slider->setSize(newSize.x - (segmentListRightBoundary + GUI_PADDING_X * 2), TEXT_BOX_HEIGHT);
		tfvInt1Slider->setSize(newSize.x - (segmentListRightBoundary + GUI_PADDING_X * 2), TEXT_BOX_HEIGHT);

		startTimeLabel->setPosition(segmentListRightBoundary, tgui::bindTop(segmentList));
		startTime->setPosition(segmentListRightBoundary, startTimeLabel->getPosition().y + startTimeLabel->getSize().y + GUI_LABEL_PADDING_Y);

		tfvFloat1Label->setPosition(segmentListRightBoundary, tgui::bindBottom(startTime) + GUI_PADDING_Y);
		tfvFloat1Slider->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat1Label) + GUI_LABEL_PADDING_Y);

		tfvFloat2Label->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat1Slider) + GUI_PADDING_Y);
		tfvFloat2Slider->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat2Label) + GUI_LABEL_PADDING_Y);

		tfvFloat3Label->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat2Slider) + GUI_PADDING_Y);
		tfvFloat3Slider->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat3Label) + GUI_LABEL_PADDING_Y);

		tfvFloat4Label->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat3Slider) + GUI_PADDING_Y);
		tfvFloat4Slider->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat4Label) + GUI_LABEL_PADDING_Y);

		tfvInt1Label->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvFloat2Slider) + GUI_PADDING_Y);
		tfvInt1Slider->setPosition(segmentListRightBoundary, tgui::bindBottom(tfvInt1Label) + GUI_LABEL_PADDING_Y);

		segmentList->setSize(segmentList->getSize().x, tgui::bindBottom(this->tfvInt1Slider));
		this->setSize(this->getSizeLayout().x, changeSegmentType->getPosition().y + changeSegmentType->getSize().y);
		ignoreResizeSignal = false;
	});

	deselectSegment();
}

void TFVGroup::setTFV(std::shared_ptr<TFV> tfv, float tfvLifespan) {
	oldTFV = tfv;
	this->tfvLifespan = tfvLifespan;
	if (dynamic_cast<PiecewiseTFV*>(tfv.get()) != nullptr) {
		this->tfv = std::dynamic_pointer_cast<PiecewiseTFV>(tfv->clone());
	} else {
		this->tfv = std::make_shared<PiecewiseTFV>();
		this->tfv->insertSegment(0, std::make_pair(0, tfv->clone()), tfvLifespan);
	}

	populateSegmentList();
	// Reselect segment to set widget values
	selectSegment(selectedSegmentIndex);
}

tgui::Signal & TFVGroup::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName())) {
		return onValueChange;
	}
	return tgui::Group::getSignal(signalName);
}

void TFVGroup::deselectSegment() {
	selectedSegmentIndex = -1;
	selectedSegment = nullptr;
	deleteSegment->setEnabled(false);
	changeSegmentType->setVisible(false);
	segmentList->getListBox()->deselectItem();
	bool f1 = false, f2 = false, f3 = false, f4 = false, i1 = false;
	tfvFloat1Label->setVisible(f1);
	tfvFloat1Slider->setVisible(f1);
	tfvFloat2Label->setVisible(f2);
	tfvFloat2Slider->setVisible(f2);
	tfvFloat3Label->setVisible(f3);
	tfvFloat3Slider->setVisible(f3);
	tfvFloat4Label->setVisible(f4);
	tfvFloat4Slider->setVisible(f4);
	tfvInt1Label->setVisible(i1);
	tfvInt1Slider->setVisible(i1);
	startTimeLabel->setVisible(false);
	startTime->setVisible(false);
}

void TFVGroup::selectSegment(int index) {
	if (index == -1 || index >= tfv->getSegmentsCount()) {
		deselectSegment();
		return;
	}

	std::lock_guard<std::recursive_mutex> lock(tfvMutex);

	selectedSegmentIndex = index;
	selectedSegment = tfv->getSegment(index).second;
	// Cannot delete the last segment
	deleteSegment->setEnabled(tfv->getSegmentsCount() != 1);
	changeSegmentType->setVisible(true);

	ignoreSignals = true;
	segmentList->getListBox()->setSelectedItemById(std::to_string(index));

	// whether to use tfvFloat1Slider, tfvFloat2Slider, ...
	bool f1 = false, f2 = false, f3 = false, f4 = false, i1 = false;

	if (dynamic_cast<LinearTFV*>(selectedSegment.get()) != nullptr) {
		f1 = f2 = true;

		tfvFloat1Label->setText("Start value");
		tfvFloat2Label->setText("End value");

		auto ptr = dynamic_cast<LinearTFV*>(selectedSegment.get());
		tfvFloat1Slider->setValue(ptr->getStartValue());
		tfvFloat2Slider->setValue(ptr->getEndValue());
	} else if (dynamic_cast<ConstantTFV*>(selectedSegment.get()) != nullptr) {
		f1 = true;

		tfvFloat1Label->setText("Value");

		auto ptr = dynamic_cast<ConstantTFV*>(selectedSegment.get());
		tfvFloat1Slider->setValue(ptr->getValue());
	} else if (dynamic_cast<SineWaveTFV*>(selectedSegment.get()) != nullptr) {
		f1 = f2 = f3 = f4 = true;

		tfvFloat1Label->setText("Period");
		tfvFloat2Label->setText("Amplitude");
		tfvFloat3Label->setText("ValueShift");
		tfvFloat4Label->setText("PhaseShift");

		auto ptr = dynamic_cast<SineWaveTFV*>(selectedSegment.get());
		tfvFloat1Slider->setValue(ptr->getPeriod());
		tfvFloat2Slider->setValue(ptr->getAmplitude());
		tfvFloat3Slider->setValue(ptr->getValueShift());
		tfvFloat4Slider->setValue(ptr->getPhaseShift());
	} else if (dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get()) != nullptr) {
		f1 = f2 = f3 = true;

		tfvFloat1Label->setText("Initial distance");
		tfvFloat2Label->setText("Initial velocity");
		tfvFloat3Label->setText("Acceleration");

		auto ptr = dynamic_cast<ConstantAccelerationDistanceTFV*>(selectedSegment.get());
		tfvFloat1Slider->setValue(ptr->getInitialDistance());
		tfvFloat2Slider->setValue(ptr->getInitialVelocity());
		tfvFloat3Slider->setValue(ptr->getAcceleration());
	} else if (dynamic_cast<DampenedStartTFV*>(selectedSegment.get()) != nullptr) {
		f1 = f2 = i1 = true;

		tfvFloat1Label->setText("Start value");
		tfvFloat2Label->setText("End value");
		tfvInt1Label->setText("Dampening factor");

		auto ptr = dynamic_cast<DampenedStartTFV*>(selectedSegment.get());
		tfvFloat1Slider->setValue(ptr->getStartValue());
		tfvFloat2Slider->setValue(ptr->getEndValue());
		tfvInt1Slider->setValue(ptr->getDampeningFactor());
	} else if (dynamic_cast<DampenedEndTFV*>(selectedSegment.get()) != nullptr) {
		f1 = f2 = i1 = true;

		tfvFloat1Label->setText("Start value");
		tfvFloat2Label->setText("End value");
		tfvInt1Label->setText("Dampening factor");

		auto ptr = dynamic_cast<DampenedEndTFV*>(selectedSegment.get());
		tfvFloat1Slider->setValue(ptr->getStartValue());
		tfvFloat2Slider->setValue(ptr->getEndValue());
		tfvInt1Slider->setValue(ptr->getDampeningFactor());
	} else if (dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get()) != nullptr) {
		f1 = f2 = i1 = true;

		tfvFloat1Label->setText("Start value");
		tfvFloat2Label->setText("End value");
		tfvInt1Label->setText("Dampening factor");

		auto ptr = dynamic_cast<DoubleDampenedTFV*>(selectedSegment.get());
		tfvFloat1Slider->setValue(ptr->getStartValue());
		tfvFloat2Slider->setValue(ptr->getEndValue());
		tfvInt1Slider->setValue(ptr->getDampeningFactor());
	}
	startTime->setValue(tfv->getSegment(index).first);
	startTime->setMin(0);
	startTime->setMax(tfvLifespan);
	//TODO: add to this if more TFVs are made

	tfvFloat1Label->setVisible(f1);
	tfvFloat1Slider->setVisible(f1);
	tfvFloat2Label->setVisible(f2);
	tfvFloat2Slider->setVisible(f2);
	tfvFloat3Label->setVisible(f3);
	tfvFloat3Slider->setVisible(f3);
	tfvFloat4Label->setVisible(f4);
	tfvFloat4Slider->setVisible(f4);
	tfvInt1Label->setVisible(i1);
	tfvInt1Slider->setVisible(i1);
	// Cannot change start time of the first segment
	startTimeLabel->setVisible(index != 0);
	startTime->setVisible(index != 0);

	ignoreSignals = false;
}

void TFVGroup::populateSegmentList() {
	ignoreSignals = true;
	std::lock_guard<std::recursive_mutex> lock(tfvMutex);
	std::string selectedIndexString = segmentList->getListBox()->getSelectedItemId();
	segmentList->getListBox()->removeAllItems();
	int index = 0;
	for (int i = 0; i < tfv->getSegmentsCount(); i++) {
		auto segment = tfv->getSegment(i);

		float nextTime = tfvLifespan;
		if (i + 1 < tfv->getSegmentsCount()) {
			nextTime = tfv->getSegment(i + 1).first;
		}

		segmentList->getListBox()->addItem("[" + std::to_string(index + 1) + "] " + segment.second->getName() + " (t=" + formatNum(segment.first) + " to t=" + formatNum(nextTime) + ")", std::to_string(index));
		index++;
	}
	// Reselect old segment if any
	if (selectedIndexString != "") {
		segmentList->getListBox()->setSelectedItemById(selectedIndexString);
	}
	ignoreSignals = false;
}

EMPAAngleOffsetGroup::EMPAAngleOffsetGroup(EditorWindow & parentWindow) : parentWindow(parentWindow) {
	changeType = tgui::Button::create();
	changeType->setText("Change type");
	changeType->connect("Pressed", [&]() {
		parentWindow.addPopupWidget(typePopup, parentWindow.getMousePos().x, parentWindow.getMousePos().y, 200, typePopup->getSize().y);
	});
	add(changeType);

	typePopup = createMenuPopup({
		std::make_pair("No offset", [&]() {
			this->offset = std::make_shared<EMPAAngleOffsetZero>();
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
			updateWidgets();
		}),
		std::make_pair("Relative to player", [&]() {
			this->offset = std::make_shared<EMPAAngleOffsetToPlayer>();
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
			updateWidgets();
		}),
		std::make_pair("Absolute position", [&]() {
			this->offset = std::make_shared<EMPAAngleOffsetToGlobalPosition>();
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
			updateWidgets();
		}),
		std::make_pair("Bind to player's direction", [&]() {
			this->offset = std::make_shared<EMPAngleOffsetPlayerSpriteAngle>();
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
			updateWidgets();
		})
	});
	typePopup->setPosition(tgui::bindLeft(changeType), tgui::bindTop(changeType));

	offsetName = tgui::Label::create();
	xLabel = tgui::Label::create();
	yLabel = tgui::Label::create();
	offsetName->setTextSize(TEXT_SIZE);
	xLabel->setTextSize(TEXT_SIZE);
	yLabel->setTextSize(TEXT_SIZE);
	add(offsetName);
	add(xLabel);
	add(yLabel);

	x = NumericalEditBoxWithLimits::create();
	y = NumericalEditBoxWithLimits::create();
	x->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) return;

		if (dynamic_cast<EMPAAngleOffsetToPlayer*>(offset.get()) != nullptr) {
			auto ptr = dynamic_cast<EMPAAngleOffsetToPlayer*>(offset.get());
			ptr->setXOffset(value);
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
		} else if (dynamic_cast<EMPAAngleOffsetToGlobalPosition*>(offset.get()) != nullptr) {
			auto ptr = dynamic_cast<EMPAAngleOffsetToGlobalPosition*>(offset.get());
			ptr->setX(value);
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
		}
	});
	y->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) return;

		if (dynamic_cast<EMPAAngleOffsetToPlayer*>(offset.get()) != nullptr) {
			auto ptr = dynamic_cast<EMPAAngleOffsetToPlayer*>(offset.get());
			ptr->setYOffset(value);
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
		} else if (dynamic_cast<EMPAAngleOffsetToGlobalPosition*>(offset.get()) != nullptr) {
			auto ptr = dynamic_cast<EMPAAngleOffsetToGlobalPosition*>(offset.get());
			ptr->setY(value);
			onValueChange.emit(this, std::make_pair(oldOffset, offset));
		}
	});
	x->setTextSize(TEXT_SIZE);
	y->setTextSize(TEXT_SIZE);
	add(x);
	add(y);

	connect("SizeChanged", [&](sf::Vector2f newSize) {
		if (ignoreResizeSignal) {
			return;
		}

		ignoreResizeSignal = true;

		offsetName->setPosition(0, 0);

		changeType->setSize(100, TEXT_BUTTON_HEIGHT);
		changeType->setPosition(tgui::bindLeft(offsetName), tgui::bindBottom(offsetName) + GUI_PADDING_Y);

		xLabel->setPosition(tgui::bindLeft(offsetName), tgui::bindBottom(changeType) + GUI_PADDING_Y);
		x->setPosition(tgui::bindLeft(offsetName), tgui::bindBottom(xLabel) + GUI_LABEL_PADDING_Y);
		yLabel->setPosition(tgui::bindLeft(offsetName), tgui::bindBottom(x) + GUI_PADDING_Y);
		y->setPosition(tgui::bindLeft(offsetName), tgui::bindBottom(yLabel) + GUI_LABEL_PADDING_Y);

		this->setSize(this->getSizeLayout().x, y->getPosition().y + y->getSize().y);
		ignoreResizeSignal = false;
	});
}

void EMPAAngleOffsetGroup::setEMPAAngleOffset(std::shared_ptr<EMPAAngleOffset> offset) {
	oldOffset = offset;
	this->offset = offset->clone();

	updateWidgets();
}

tgui::Signal & EMPAAngleOffsetGroup::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName())) {
		return onValueChange;
	}
	return tgui::Group::getSignal(signalName);
}

void EMPAAngleOffsetGroup::updateWidgets() {
	ignoreSignals = true;

	offsetName->setText(offset->getName());

	if (dynamic_cast<EMPAAngleOffsetToPlayer*>(offset.get()) != nullptr) {
		auto ptr = dynamic_cast<EMPAAngleOffsetToPlayer*>(offset.get());
		xLabel->setText("X offset");
		yLabel->setText("Y offset");
		x->setValue(ptr->getXOffset());
		y->setValue(ptr->getYOffset());

		xLabel->setVisible(true);
		yLabel->setVisible(true);
		x->setVisible(true);
		y->setVisible(true);
	} else if (dynamic_cast<EMPAAngleOffsetToGlobalPosition*>(offset.get()) != nullptr) {
		auto ptr = dynamic_cast<EMPAAngleOffsetToGlobalPosition*>(offset.get());
		xLabel->setText("X");
		yLabel->setText("Y");
		x->setValue(ptr->getX());
		y->setValue(ptr->getY());

		xLabel->setVisible(true);
		yLabel->setVisible(true);
		x->setVisible(true);
		y->setVisible(true);
	} else if (dynamic_cast<EMPAAngleOffsetZero*>(offset.get()) != nullptr) {
		xLabel->setVisible(false);
		yLabel->setVisible(false);
		x->setVisible(false);
		y->setVisible(false);
	} else if (dynamic_cast<EMPAngleOffsetPlayerSpriteAngle*>(offset.get()) != nullptr) {
		xLabel->setVisible(false);
		yLabel->setVisible(false);
		x->setVisible(false);
		y->setVisible(false);
	} else {
		// You forgot a case
		assert(false);
	}

	ignoreSignals = false;
}

ListBoxScrollablePanel::ListBoxScrollablePanel() {
	listBox = tgui::ListBox::create();
	add(listBox);
	listBox->setSize("100%", "100%");

	textWidthChecker = tgui::Label::create();
	textWidthChecker->setMaximumTextWidth(0);

	connect("SizeChanged", [&]() {
		onListBoxItemsUpdate();
	});
}

void ListBoxScrollablePanel::onListBoxItemsUpdate() {
	float largestWidth = 0;
	for (auto str : listBox->getItems()) {
		textWidthChecker->setText(str);
		largestWidth = std::max(largestWidth, textWidthChecker->getSize().x);
	}
	listBox->setSize(std::max(getSize().x, largestWidth), "100%");
}

bool ListBoxScrollablePanel::mouseWheelScrolled(float delta, tgui::Vector2f pos) {
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

void ListBoxScrollablePanel::setTextSize(int textSize) {
	listBox->setTextSize(textSize);
	textWidthChecker->setTextSize(textSize);
}

ListViewScrollablePanel::ListViewScrollablePanel() {
	listView = tgui::ListView::create();
	add(listView);
	listView->setSize("100%", "100%");

	textWidthChecker = tgui::Label::create();
	textWidthChecker->setMaximumTextWidth(0);

	connect("SizeChanged", [&]() {
		onListViewItemsUpdate();
	});
}

void ListViewScrollablePanel::onListViewItemsUpdate() {
	float largestWidth = 0;
	for (auto str : listView->getItems()) {
		textWidthChecker->setText(str);
		largestWidth = std::max(largestWidth, textWidthChecker->getSize().x);
	}
	listView->setSize(std::max(getSize().x, largestWidth), "100%");
}

bool ListViewScrollablePanel::mouseWheelScrolled(float delta, tgui::Vector2f pos) {
	// This override makes it so if mouse wheel is scrolled when the scrollbar can't be scrolled any more,
	// the event is not consumed so that this widget's parent can handle the event.

	if (delta < 0) {
		// Super ghetto way of checking if scroll is already at max
		auto oldScrollbarValue = listView->getVerticalScrollbarValue();
		listView->setVerticalScrollbarValue(2000000000);
		if (listView->getVerticalScrollbarValue() == oldScrollbarValue) {
			listView->setVerticalScrollbarValue(oldScrollbarValue);
			return false;
		} else {
			listView->setVerticalScrollbarValue(oldScrollbarValue);
			return tgui::ScrollablePanel::mouseWheelScrolled(delta, pos);
		}
	} else if (listView->getVerticalScrollbarValue() == 0) {
		return false;
	} else {
		return tgui::ScrollablePanel::mouseWheelScrolled(delta, pos);
	}
}

void ListViewScrollablePanel::setTextSize(int textSize) {
	listView->setTextSize(textSize);
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

SimpleEngineRenderer::SimpleEngineRenderer(sf::RenderWindow & parentWindow, bool userControlledView, bool useDebugRenderSystem) : parentWindow(parentWindow), 
paused(true), userControlledView(userControlledView), useDebugRenderSystem(useDebugRenderSystem) {
	audioPlayer = std::make_unique<AudioPlayer>();
	queue = std::make_unique<EntityCreationQueue>(registry);

	if (userControlledView) {
		viewController = std::make_unique<ViewController>(parentWindow);
	}

	connect("PositionChanged", [&]() {
		updateWindowView();
	});
	connect("SizeChanged", [&]() {
		updateWindowView();
	});
}

void SimpleEngineRenderer::updateWindowView() {
	if (!renderSystem) {
		return;
	}

	auto windowSize = parentWindow.getSize();
	auto size = getSize();
	float sizeRatio = size.x / (float)size.y;
	sf::Vector2u resolution = renderSystem->getResolution();
	float playAreaViewRatio = resolution.x / (float)resolution.y;

	float viewWidth, viewHeight;
	if (sizeRatio > playAreaViewRatio) {
		viewHeight = resolution.y;
		viewWidth = resolution.y * size.x / (float)size.y;
		float viewX = -(viewWidth - resolution.x) / 2.0f;
		float viewY = 0;
		viewFloatRect = sf::FloatRect(viewX, viewY, viewWidth, viewHeight);
	} else {
		viewWidth = resolution.x;
		viewHeight = resolution.x * size.y / (float)size.x;
		float viewX = 0;
		float viewY = -(viewHeight - resolution.y) / 2.0f;
		viewFloatRect = sf::FloatRect(viewX, viewY, viewWidth, viewHeight);
	}

	if (viewController) {
		viewController->setOriginalViewSize(viewWidth, viewHeight);
	}

	float viewportX = getAbsolutePosition().x / windowSize.x;
	float viewportY = getAbsolutePosition().y / windowSize.y;
	float viewportWidth = getSize().x / windowSize.x;
	float viewportHeight = getSize().y / windowSize.y;
	viewportFloatRect = sf::FloatRect(viewportX, viewportY, viewportWidth, viewportHeight);

	viewFromViewController = sf::View(viewFloatRect);
	viewFromViewController.setViewport(viewportFloatRect);
}

void SimpleEngineRenderer::draw(sf::RenderTarget & target, sf::RenderStates states) const {
	tgui::Panel::draw(target, states);

	sf::Clock deltaClock;
	float secondsSinceLastRender = 0;
	while (secondsSinceLastRender < RENDER_INTERVAL) {
		float dt = std::min(MAX_PHYSICS_DELTA_TIME, deltaClock.restart().asSeconds());
		physicsUpdate(dt);
		secondsSinceLastRender += dt;
	}

	// Viewport is set here because tgui::Gui's draw function changes it right before renderSystem is updated or something
	sf::View originalView = parentWindow.getView();
	if (viewController) {
		parentWindow.setView(viewFromViewController);
	} else {
		sf::View view(viewFloatRect);
		view.setViewport(viewportFloatRect);
		parentWindow.setView(view);
	}
	if (!paused) {
		spriteAnimationSystem->update(secondsSinceLastRender);
	}
	renderSystem->update(secondsSinceLastRender);
	parentWindow.setView(originalView);
}

bool SimpleEngineRenderer::handleEvent(sf::Event event) {
	if (viewController) {
		return viewController->handleEvent(viewFromViewController, event);
	}
	return false;
}

void SimpleEngineRenderer::loadLevelPack(std::string name) {
	registry.reset();

	levelPack = std::make_shared<LevelPack>(*audioPlayer, name);
	spriteLoader = levelPack->createSpriteLoader();
	spriteLoader->preloadTextures();

	movementSystem = std::make_unique<MovementSystem>(*queue, *spriteLoader, registry);
	//TODO: these numbers should come from settings
	if (useDebugRenderSystem) {
		renderSystem = std::make_unique<DebugRenderSystem>(registry, parentWindow, *spriteLoader, 1.0f);
	} else {
		renderSystem = std::make_unique<RenderSystem>(registry, parentWindow, *spriteLoader, 1.0f);
	}
	collisionSystem = std::make_unique<CollisionSystem>(*levelPack, *queue, *spriteLoader, registry, MAP_WIDTH, MAP_HEIGHT);
	despawnSystem = std::make_unique<DespawnSystem>(registry);
	enemySystem = std::make_unique<EnemySystem>(*queue, *spriteLoader, *levelPack, registry);
	spriteAnimationSystem = std::make_unique<SpriteAnimationSystem>(*spriteLoader, registry);
	shadowTrailSystem = std::make_unique<ShadowTrailSystem>(*queue, registry);
	playerSystem = std::make_unique<PlayerSystem>(*levelPack, *queue, *spriteLoader, registry);
	collectibleSystem = std::make_unique<CollectibleSystem>(*queue, registry, *levelPack, MAP_WIDTH, MAP_HEIGHT);

	renderSystem->getOnResolutionChange()->sink().connect<SimpleEngineRenderer, &SimpleEngineRenderer::updateWindowView>(this);
	updateWindowView();
}

void SimpleEngineRenderer::loadLevelPack(std::shared_ptr<LevelPack> levelPack) {
	this->levelPack = levelPack;
	registry.reset();

	movementSystem = std::make_unique<MovementSystem>(*queue, *spriteLoader, registry);
	//TODO: these numbers should come from settings
	if (useDebugRenderSystem) {
		renderSystem = std::make_unique<DebugRenderSystem>(registry, parentWindow, *spriteLoader, 1.0f);
	} else {
		renderSystem = std::make_unique<RenderSystem>(registry, parentWindow, *spriteLoader, 1.0f);
	}
	collisionSystem = std::make_unique<CollisionSystem>(*levelPack, *queue, *spriteLoader, registry, MAP_WIDTH, MAP_HEIGHT);
	despawnSystem = std::make_unique<DespawnSystem>(registry);
	enemySystem = std::make_unique<EnemySystem>(*queue, *spriteLoader, *levelPack, registry);
	spriteAnimationSystem = std::make_unique<SpriteAnimationSystem>(*spriteLoader, registry);
	shadowTrailSystem = std::make_unique<ShadowTrailSystem>(*queue, registry);
	playerSystem = std::make_unique<PlayerSystem>(*levelPack, *queue, *spriteLoader, registry);
	collectibleSystem = std::make_unique<CollectibleSystem>(*queue, registry, *levelPack, MAP_WIDTH, MAP_HEIGHT);

	renderSystem->getOnResolutionChange()->sink().connect<SimpleEngineRenderer, &SimpleEngineRenderer::updateWindowView>(this);
	updateWindowView();
}

void SimpleEngineRenderer::loadLevel(int levelIndex) {
	if (!levelPack->hasLevel(levelIndex)) {
		throw "The level does not exist";
	}
	loadLevel(levelPack->getLevel(levelIndex));
}

void SimpleEngineRenderer::loadLevel(std::shared_ptr<Level> level) {
	// Load bloom settings
	renderSystem->loadLevelRenderSettings(level);

	// Remove all existing entities from the registry
	registry.reset();
	reserveMemory(registry, INITIAL_EDITOR_ENTITY_RESERVATION);

	// Create the level manager
	registry.reserve<LevelManagerTag>(1);
	registry.reserve(registry.alive() + 1);
	uint32_t levelManager = registry.create();
	auto& levelManagerTag = registry.assign<LevelManagerTag>(entt::tag_t{}, levelManager, &(*levelPack), level);

	// Create the player
	auto params = levelPack->getPlayer();
	registry.reserve(1);
	registry.reserve<PlayerTag>(1);
	registry.reserve<AnimatableSetComponent>(1);
	registry.reserve<HealthComponent>(1);
	registry.reserve<HitboxComponent>(1);
	registry.reserve<PositionComponent>(1);
	registry.reserve<SpriteComponent>(1);
	auto player = registry.create();
	registry.assign<AnimatableSetComponent>(player);
	auto& playerTag = registry.assign<PlayerTag>(entt::tag_t{}, player, registry, *levelPack, player, params->getSpeed(), params->getFocusedSpeed(), params->getInvulnerabilityTime(),
		params->getPowerTiers(), params->getHurtSound(), params->getDeathSound(), params->getInitialBombs(), params->getMaxBombs(), params->getBombInvincibilityTime());
	auto& health = registry.assign<HealthComponent>(player, params->getInitialHealth(), params->getMaxHealth());
	// Hitbox temporarily at 0, 0 until an Animatable is assigned to the player later
	registry.assign<HitboxComponent>(player, LOCK_ROTATION, params->getHitboxRadius(), 0, 0);
	registry.assign<PositionComponent>(player, PLAYER_SPAWN_X - params->getHitboxPosX(), PLAYER_SPAWN_Y - params->getHitboxPosY());
	registry.assign<SpriteComponent>(player, PLAYER_LAYER, 0);

	// Play level music
	levelPack->playMusic(level->getMusicSettings());

	// Set the background
	std::string backgroundFileName = level->getBackgroundFileName();
	sf::Texture background;
	if (!background.loadFromFile("Level Packs\\" + levelPack->getName() + "\\Backgrounds\\" + backgroundFileName)) {
		//TODO: load a default background
	}
	background.setRepeated(true);
	background.setSmooth(true);
	renderSystem->setBackground(std::move(background));
	renderSystem->setBackgroundScrollSpeedX(level->getBackgroundScrollSpeedX());
	renderSystem->setBackgroundScrollSpeedY(level->getBackgroundScrollSpeedY());
	renderSystem->setBackgroundTextureWidth(level->getBackgroundTextureWidth());
	renderSystem->setBackgroundTextureHeight(level->getBackgroundTextureHeight());
}

void SimpleEngineRenderer::pause() {
	paused = true;
}

void SimpleEngineRenderer::unpause() {
	paused = false;
}

void SimpleEngineRenderer::physicsUpdate(float deltaTime) const {
	if (!paused) {
		audioPlayer->update(deltaTime);

		collisionSystem->update(deltaTime);
		queue->executeAll();

		registry.get<LevelManagerTag>().update(*queue, *spriteLoader, registry, deltaTime);
		queue->executeAll();

		despawnSystem->update(deltaTime);
		queue->executeAll();

		shadowTrailSystem->update(deltaTime);
		queue->executeAll();

		movementSystem->update(deltaTime);
		queue->executeAll();

		collectibleSystem->update(deltaTime);
		queue->executeAll();

		playerSystem->update(deltaTime);
		queue->executeAll();

		enemySystem->update(deltaTime);
		queue->executeAll();
	}
}

TabsWithPanel::TabsWithPanel(EditorWindow& parentWindow) : parentWindow(parentWindow) {	
	tabs = tgui::Tabs::create();

	tabsContainer = tgui::ScrollablePanel::create();
	tabsContainer->setPosition(0, 0);
	add(tabsContainer);
	
	tabs->setPosition(0, 0);
	tabs->connect("TabSelected", &TabsWithPanel::onTabSelected, this);
	tabs->setTextSize(TEXT_SIZE);
	// Clamp tab size
	tabs->setMinimumTabWidth(TAB_WIDTH);
	tabs->setMaximumTabWidth(TAB_WIDTH);
	tabsContainer->add(tabs);
	// Make room in the height for the scrollbar
	tabsContainer->setSize("100%", tgui::bindHeight(tabs) + 30);

	// Make room for the close button for every tab
	tabNameAppendedSpaces = std::string((int)std::ceil((float)tabs->getSize().y /tabs->getTextSize()), ' ');

	moreTabsList = ListBoxScrollablePanel::create();
	moreTabsList->setTextSize(TEXT_SIZE);
	moreTabsList->getListBox()->setItemHeight(TEXT_SIZE * 1.5f);
	moreTabsList->getListBox()->connect("ItemSelected", [&](std::string tabName) {
		// Remove spaces from the end since selectTab() readds them
		selectTab(tabName.substr(0, tabName.length() - tabNameAppendedSpaces.length()));
	});

	moreTabsButton = tgui::Button::create();
	//TODO: image on this instead of a V
	moreTabsButton->setText("V");
	moreTabsButton->connect("Pressed", [&]() {
		float preferredWidth = 300;
		float preferredHeight = (moreTabsList->getListBox()->getItemHeight() + 1) * moreTabsList->getListBox()->getItemCount();
		// Set moreTabsList's position releative to the absolute position of the moreTabsButton since it will be a top-level widget
		if (moreTabsListAlignment == MoreTabsListAlignment::Left) {
			parentWindow.addPopupWidget(moreTabsList, moreTabsButton->getAbsolutePosition().x - preferredWidth + moreTabsButton->getSize().x, moreTabsButton->getAbsolutePosition().y + moreTabsButton->getSize().y, preferredWidth, preferredHeight);
		} else if (moreTabsListAlignment == MoreTabsListAlignment::Right) {
			parentWindow.addPopupWidget(moreTabsList, moreTabsButton->getAbsolutePosition().x, moreTabsButton->getAbsolutePosition().y + moreTabsButton->getSize().y, preferredWidth, preferredHeight);
		} else {
			// You forgot a case
			assert(false);
		}
	});
	add(moreTabsButton);

	connect("SizeChanged", [&](sf::Vector2f newSize) {
		moreTabsButton->setSize(tabs->getSize().y, tabs->getSize().y);
		moreTabsButton->setPosition(getSize().x - moreTabsButton->getSize().x, tgui::bindTop(tabs));
	
		// Update height of the tabs container
		if (tabs->getSize().x > getSize().x - moreTabsButton->getSize().x) {
			// If tabs's width > this widget's parent's width, the scrollbar is visible,
			// so make room for the scrollbar
			tabsContainer->setSize(getSize().x - moreTabsButton->getSize().x, tgui::bindHeight(tabs) + EXTRA_HEIGHT_FROM_SCROLLBAR);
		} else {
			// The scrollbar is not visible, so don't make room for the scrollbar
			tabsContainer->setSize(getSize().x - moreTabsButton->getSize().x, tgui::bindHeight(tabs));
		}
	});

	onTabsChange();
}

void TabsWithPanel::addTab(std::string tabName, std::shared_ptr<tgui::Panel> associatedPanel, bool autoSelect, bool closeable) {
	tabName += tabNameAppendedSpaces;
	if (panelsMap.count(tabName) > 0) {
		return;
	}

	panelsMap[tabName] = associatedPanel;
	associatedPanel->setPosition(0, tgui::bindBottom(tabsContainer));
	associatedPanel->setSize("100%", tgui::bindHeight(shared_from_this()) - tgui::bindBottom(tabsContainer));
	associatedPanel->setVisible(autoSelect);
	tabs->add(tabName, autoSelect);
	add(associatedPanel);

	tabsOrdering.push_back(tabName);
	if (closeable) {
		createCloseButton(tabs->getTabsCount() - 1);
	} else {
		closeButtons.push_back(std::make_pair(nullptr, ""));
	}
	onTabsChange();
}

void TabsWithPanel::insertTab(std::string tabName, std::shared_ptr<tgui::Panel> associatedPanel, int index, bool autoSelect, bool closeable) {
	tabName += tabNameAppendedSpaces;
	if (panelsMap.count(tabName) > 0) {
		return;
	}

	panelsMap[tabName] = associatedPanel;
	associatedPanel->setPosition(0, tgui::bindBottom(tabsContainer));
	associatedPanel->setSize("100%", tgui::bindHeight(shared_from_this()) - tgui::bindBottom(tabsContainer));
	associatedPanel->setVisible(autoSelect);
	tabs->insert(index, tabName, autoSelect);
	add(associatedPanel);

	tabsOrdering.insert(tabsOrdering.begin(), tabName);
	if (closeable) {
		createCloseButton(index);
	} else {
		closeButtons.push_back(std::make_pair(nullptr, ""));
	}
	onTabsChange();
}

void TabsWithPanel::selectTab(std::string tabName) {
	if (tabName == "") {
		return;
	}
	tabName += tabNameAppendedSpaces;

	tabs->select(tabName);
}

void TabsWithPanel::removeTab(std::string tabName) {
	tabName += tabNameAppendedSpaces;
	assert(panelsMap.count(tabName) != 0);

	remove(panelsMap[tabName]);
	if (currentPanel == panelsMap[tabName]) {
		currentPanel = nullptr;
	}
	panelsMap.erase(tabName);
	tabs->remove(tabName);

	int pos;
	for (pos = 0; pos < tabsOrdering.size(); pos++) {
		if (tabsOrdering[pos] == tabName) break;
	}
	tabsOrdering.erase(tabsOrdering.begin() + pos);
	// Remove the tab's close button
	tabsContainer->remove(closeButtons[pos].first);
	closeButtons.erase(closeButtons.begin() + pos);
	// Reposition all close buttons that comee after the removed one
	for (int i = pos; i < tabsOrdering.size(); i++) {
		closeButtons[i].first->setPosition((i + 1) * (TAB_WIDTH + 1) - closeButtons[i].first->getSize().x, tgui::bindTop(tabs));
	}

	onTabsChange();
}

void TabsWithPanel::removeAllTabs() {
	for (auto it = panelsMap.begin(); it != panelsMap.end(); it++) {
		remove(it->second);
	}
	panelsMap.clear();
	tabsOrdering.clear();
	for (std::pair<std::shared_ptr<tgui::Button>, std::string> closeButtonAndPrompt : closeButtons) {
		currentPanel->remove(closeButtonAndPrompt.first);
	}
	closeButtons.clear();
	currentPanel = nullptr;
	tabs->removeAll();

	onTabsChange();
}

void TabsWithPanel::setTabCloseButtonConfirmationPrompt(std::string tabName, std::string message) {
	tabName += tabNameAppendedSpaces;
	int pos;
	for (pos = 0; pos < tabsOrdering.size(); pos++) {
		if (tabsOrdering[pos] == tabName) break;
	}
	setTabCloseButtonConfirmationPrompt(pos, message);
}

bool TabsWithPanel::hasTab(std::string tabName) {
	return panelsMap.count(tabName + tabNameAppendedSpaces) > 0;
}

void TabsWithPanel::cacheTabs(std::string tabsSetIdentifier) {
	std::vector<std::pair<std::string, std::shared_ptr<tgui::Panel>>> tabsData;
	for (auto tabName : tabsOrdering) {
		tabsData.push_back(std::make_pair(tabName, panelsMap[tabName]));
	}
	std::vector<std::string> closeButtonPrompts;
	for (std::pair<std::shared_ptr<tgui::Button>, std::string> buttonAndPrompt : closeButtons) {
		closeButtonPrompts.push_back(buttonAndPrompt.second);
	}
	tabsCache.insert(tabsSetIdentifier, { tabs->getSelected(), tabsData, closeButtonPrompts });
}

bool TabsWithPanel::isCached(std::string tabsSetIdentifier) {
	return tabsCache.contains(tabsSetIdentifier);
}

bool TabsWithPanel::loadCachedTabsSet(std::string tabsSetIdentifier) {
	if (!tabsCache.contains(tabsSetIdentifier)) {
		return false;
	}

	removeAllTabs();

	CachedTabsValue data = tabsCache.get(tabsSetIdentifier);
	for (std::pair<std::string, std::shared_ptr<tgui::Panel>> p : data.tabs) {
		addTab(p.first, p.second, p.first == data.currentlySelectedTab);
	}
	int i = 0;
	for (std::string message : data.closeButtonConfirmationPrompts) {
		setTabCloseButtonConfirmationPrompt(i, message);
		i++;
	}
	onTabsChange();
}

void TabsWithPanel::removeCachedTabsSet(std::string tabsSetIdentifier) {
	if (tabsCache.contains(tabsSetIdentifier)) {
		tabsCache.remove(tabsSetIdentifier);
	}
}

void TabsWithPanel::clearTabsCache() {
	tabsCache.clear();
}

tgui::Signal & TabsWithPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onTabClose.getName())) {
		return onTabClose;
	}
	return tgui::Group::getSignal(signalName);
}

std::string TabsWithPanel::getSelectedTab() {
	return tabs->getSelected();
}

void TabsWithPanel::setMoreTabsListAlignment(MoreTabsListAlignment moreTabsListAlignment) {
	this->moreTabsListAlignment = moreTabsListAlignment;
}

bool TabsWithPanel::handleEvent(sf::Event event) {
	// Let the currently opened panel handle the event, if it can
	if (currentPanel) {
		std::shared_ptr<EventCapturable> eventCapturer = std::dynamic_pointer_cast<EventCapturable>(currentPanel);
		if (eventCapturer) {
			return eventCapturer->handleEvent(event);
		}
	}
	return false;
}

void TabsWithPanel::onTabSelected(std::string tabName) {
	assert(panelsMap.count(tabName) != 0);

	// Hide currently open panel
	if (currentPanel) {
		currentPanel->setVisible(false);
	}

	// Show the selected tab's panel
	currentPanel = panelsMap[tabName];
	currentPanel->setVisible(true);

	moreTabsList->getListBox()->setSelectedItem(tabName);
}

void TabsWithPanel::onTabsChange() {
	moreTabsList->getListBox()->removeAllItems();
	for (int i = 0; i < tabsOrdering.size(); i++) {
		moreTabsList->getListBox()->addItem(tabsOrdering[i], tabsOrdering[i]);
	}

	// More tabs button only visible if there are more than 0 tabs
	moreTabsButton->setVisible(tabsOrdering.size() > 0);

	// Update size of tabs
	tabs->setSize(tabsOrdering.size() * tabs->getMinimumTabWidth(), tabs->getSize().y);

	// Update height of the tabs container
	if (tabs->getSize().x > getSize().x - moreTabsButton->getSize().x) {
		// The scrollbar is visible, so make room for the scrollbar
		tabsContainer->setSize(getSize().x - moreTabsButton->getSize().x, tgui::bindHeight(tabs) + EXTRA_HEIGHT_FROM_SCROLLBAR);
	} else {
		// The scrollbar is not visible, so don't make room for the scrollbar
		tabsContainer->setSize(getSize().x - moreTabsButton->getSize().x, tgui::bindHeight(tabs));
	}
}

void TabsWithPanel::onCloseButtonConfirmationPromptAnswer(bool confirmed) {
	if (confirmed) {
		removeTab(closeButtonConfirmationPromptTargetTabShortenedName);
		onTabClose.emit(this, closeButtonConfirmationPromptTargetTabShortenedName);
	}
}

void TabsWithPanel::setTabCloseButtonConfirmationPrompt(int index, std::string message) {
	closeButtons[index].second = message;
}

void TabsWithPanel::createCloseButton(int index) {
	// Create close button
	std::string tabName = tabs->getText(index);
	std::shared_ptr<tgui::Button> closeButton = tgui::Button::create();
	closeButton->setSize(tabs->getSize().y, tabs->getSize().y);
	closeButton->setTextSize(TEXT_SIZE);
	closeButton->setText("X");
	// +1 to the tab width because for some reason each tab's width is actually
	// 1 pixel more than TAB_WIDTH
	closeButton->setPosition((index + 1) * (TAB_WIDTH + 1) - closeButton->getSize().x, tgui::bindTop(tabs));
	closeButton->connect("Pressed", [&, tabName]() {
		int pos;
		for (pos = 0; pos < tabsOrdering.size(); pos++) {
			if (tabsOrdering[pos] == tabName) break;
		}
		std::string fullTabName = tabs->getText(pos); // Includes the spaces, so remove them
		closeButtonConfirmationPromptTargetTabShortenedName = fullTabName.substr(0, fullTabName.length() - tabNameAppendedSpaces.length());
		if (closeButtons[pos].second != "") {
			parentWindow.promptConfirmation(closeButtons[pos].second)->sink().connect<TabsWithPanel, &TabsWithPanel::onCloseButtonConfirmationPromptAnswer>(this);
		} else {
			removeTab(closeButtonConfirmationPromptTargetTabShortenedName);
		}
	});
	closeButtons.push_back(std::make_pair(closeButton, ""));
	tabsContainer->add(closeButton);
}

ClickableTimeline::ClickableTimeline() {
	cameraController = tgui::RangeSlider::create();
	cameraController->setPosition(6, 6);
	cameraController->connect("RangeChanged", [&](float lower, float upper) {
		if ((upper - lower) / maxTimeValue < 0.01) {
			cameraController->setSelectionEnd(lower + maxTimeValue * 0.01);
			cameraController->setSelectionStart(upper - maxTimeValue * 0.01);
			return;
		}

		buttonScalarScalar = 1 / ((upper - lower)/maxTimeValue);
		panel->setHorizontalScrollbarValue(lower * buttonScalar * buttonScalarScalar);
		
		updateButtonsPositionsAndSizes();
	});
	add(cameraController);

	panel = tgui::ScrollablePanel::create();
	panel->setHorizontalScrollbarPolicy(tgui::Scrollbar::Policy::Never);	
	panel->setVerticalScrollbarPolicy(tgui::Scrollbar::Policy::Never);
	add(panel);

	connect("SizeChanged", [&](sf::Vector2f newSize) {
		cameraController->setSize(newSize.x - 12, SLIDER_HEIGHT);
		panel->setSize("100%", newSize.y - SLIDER_HEIGHT - 5);
		panel->setPosition(0, tgui::bindBottom(cameraController) + 5);
		buttonScalar = newSize.x / maxTimeValue;

		updateButtonsPositionsAndSizes();
	});
}

void ClickableTimeline::setElements(std::vector<std::pair<float, std::string>> elementStartTimesAndDuration, float maxTime) {
	if (elementStartTimesAndDuration.size() == 0) {
		elements.clear();
		panel->removeAllWidgets();
		return;
	}
	
	std::vector<std::tuple<float, float, std::string>> converted;
	for (int i = 1; i < elementStartTimesAndDuration.size(); i++) {
		converted.push_back(std::make_tuple(elementStartTimesAndDuration[i - 1].first, elementStartTimesAndDuration[i].first - elementStartTimesAndDuration[i - 1].first, elementStartTimesAndDuration[i - 1].second));
	}
	converted.push_back(std::make_tuple(elementStartTimesAndDuration[elementStartTimesAndDuration.size() - 1].first, maxTime - elementStartTimesAndDuration[elementStartTimesAndDuration.size() - 1].first, elementStartTimesAndDuration[elementStartTimesAndDuration.size() - 1].second));
	setElements(converted, maxTime);
}

void ClickableTimeline::setElements(std::vector<std::tuple<float, float, std::string>> elementStartTimesAndDuration) {
	elements.clear();
	panel->removeAllWidgets();

	if (elementStartTimesAndDuration.size() == 0) return;

	maxTimeValue = std::get<0>(elementStartTimesAndDuration[elementStartTimesAndDuration.size() - 1]) + std::get<1>(elementStartTimesAndDuration[elementStartTimesAndDuration.size() - 1]);
	buttonScalar = panel->getSize().x/maxTimeValue;

	cameraController->setMaximum(maxTimeValue);
	cameraController->setStep(maxTimeValue / 200.0f);
	cameraController->setSelectionStart(0);
	cameraController->setSelectionEnd(maxTimeValue);

	int i = 0;
	for (std::tuple<float, float, std::string> tuple : elementStartTimesAndDuration) {
		std::shared_ptr<tgui::Button> button = tgui::Button::create();
		button->setText(std::get<2>(tuple));
		button->connect("Pressed", [&, i]() {
			onElementPressed.emit(this, i);
		});
		panel->add(button);
		elements.push_back(std::make_tuple(std::get<0>(tuple), std::get<1>(tuple), button));
		i++;
	}

	updateButtonsPositionsAndSizes();
}

void ClickableTimeline::setElements(std::vector<std::tuple<float, float, std::string>> elementStartTimesAndDuration, float maxTime) {
	elements.clear();
	panel->removeAllWidgets();

	if (elementStartTimesAndDuration.size() == 0) return;

	maxTimeValue = maxTime;
	buttonScalar = panel->getSize().x / maxTimeValue;

	cameraController->setMaximum(maxTimeValue);
	cameraController->setStep(maxTimeValue / 200.0f);
	cameraController->setSelectionStart(0);
	cameraController->setSelectionEnd(maxTimeValue);

	int i = 0;
	for (std::tuple<float, float, std::string> tuple : elementStartTimesAndDuration) {
		std::shared_ptr<tgui::Button> button = tgui::Button::create();
		button->setText(std::get<2>(tuple));
		button->connect("Pressed", [&, i]() {
			onElementPressed.emit(this, i);
		});
		panel->add(button);
		elements.push_back(std::make_tuple(std::get<0>(tuple), std::get<1>(tuple), button));
		i++;
	}

	updateButtonsPositionsAndSizes();
}

tgui::Signal& ClickableTimeline::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onElementPressed.getName())) {
		return onElementPressed;
	}
	return tgui::Group::getSignal(signalName);
}

void ClickableTimeline::updateButtonsPositionsAndSizes() {
	for (auto tuple : elements) {
		std::shared_ptr<tgui::Button> button = std::get<2>(tuple);
		button->setPosition(std::get<0>(tuple) * buttonScalar * buttonScalarScalar, 0);
		button->setSize(std::get<1>(tuple) * buttonScalar * buttonScalarScalar, panel->getSize().y - 2);
	}
}

void TimedLabel::update(sf::Time elapsedTime) {
	timeSinceLastChar += elapsedTime.asSeconds();
	int numCharsToBeShown = (int)(timeSinceLastChar / charDelay);
	if (numCharsToBeShown > 0) {
		numVisibleChars = std::min(numVisibleChars + numCharsToBeShown, textNumChars);
		Label::setText(text.substr(0, numVisibleChars));
	}
	timeSinceLastChar = std::fmod(timeSinceLastChar, charDelay);

	Label::update(elapsedTime);
}

void TimedLabel::setText(const sf::String& text) {
	this->text = text;
	textNumChars = text.getSize();
	timeSinceLastChar = 0;
	numVisibleChars = 0;

	Label::setText("");
}

DelayedSlider::DelayedSlider() {
	// connect() will call DelayedSlider::getSignal(), so we set ignoreDelayedSliderSignalName
	// to true to get the tgui::Slider's ValueChanged signal just for this call
	ignoreDelayedSliderSignalName = true;
	connect("ValueChanged", [this]() {
		this->timeElapsedSinceLastValueChange = 0;
		this->valueChangeSignalEmitted = false;
	});
	ignoreDelayedSliderSignalName = false;
}

void DelayedSlider::update(sf::Time elapsedTime) {
	Slider::update(elapsedTime);

	timeElapsedSinceLastValueChange += elapsedTime.asSeconds();
	if (!valueChangeSignalEmitted && timeElapsedSinceLastValueChange >= VALUE_CHANGE_WINDOW) {
		valueChangeSignalEmitted = true;
		onValueChange.emit(this, getValue());
	}
}

tgui::Signal & DelayedSlider::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName()) && !ignoreDelayedSliderSignalName) {
		return onValueChange;
	}
	return Slider::getSignal(signalName);
}
