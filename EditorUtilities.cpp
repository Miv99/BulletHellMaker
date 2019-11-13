#include "EditorUtilities.h"
#include "Constants.h"
#include "EditorWindow.h"
#include <map>
#include <boost/filesystem.hpp>
#include <iostream>
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

TFVGroup::TFVGroup(std::shared_ptr<std::recursive_mutex> tguiMutex, float paddingX, float paddingY) : tguiMutex(tguiMutex), paddingX(paddingX), paddingY(paddingY), undoStack(UndoStack(50)) {
	std::lock_guard<std::recursive_mutex> lock(*tguiMutex);

	onEditingStart = std::make_shared<entt::SigH<void()>>();
	onEditingEnd = std::make_shared<entt::SigH<void(std::shared_ptr<TFV>, std::shared_ptr<TFV>)>>();

	tfvShortDescription = tgui::Label::create();
	beginEditingButton = tgui::Button::create();
	beginEditingButton->setText("Edit");

	beginEditingButton->connect("Pressed", [&]() {
		beginEditing();
	});

	add(tfvShortDescription);
	add(beginEditingButton);

	//TODO: change default size
	tfvEditorWindow = std::make_shared<UndoableEditorWindow>(tguiMutex, "TFV Editor", 1024, 768, undoStack);
	// Stop editing when the window is closed
	tfvEditorWindow->getCloseSignal()->sink().connect<TFVGroup, &TFVGroup::endEditingWithoutSaving>(this);
	
	// Add widgets to TFV editor
	panel = tgui::ScrollablePanel::create();

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
	panel->add(showGraph);

	tfvEditorWindow->getGui()->add(panel);
	tfvEditorWindow->getResizeSignal()->sink().connect<TFVGroup, &TFVGroup::onTFVEditorWindowResize>(this);
}

void TFVGroup::beginEditing() {
	// Start the window thread
	tfvEditorWindowThread = std::thread(&EditorWindow::start, tfvEditorWindow);
	tfvEditorWindowThread.detach();

	onEditingStart->publish();

	
}

void TFVGroup::endEditing(bool saveEditedTFV) {
	// Close the window and its thread
	tfvEditorWindow->close();
	// Clear its undo stack
	undoStack.clear();

	if (saveEditedTFV) {
		onEditingEnd->publish(oldTFV, tfv);
	} else {
		onEditingEnd->publish(oldTFV, oldTFV);
	}
}

void TFVGroup::endEditingWithoutSaving() {
	endEditing(false);
}

void TFVGroup::onContainerResize(int containerWidth, int containerHeight) {
	tfvShortDescription->setPosition(0, 0);
	beginEditingButton->setPosition(0, tgui::bindBottom(tfvShortDescription) + paddingY);
	beginEditingButton->setSize(100, TEXT_BUTTON_HEIGHT);

	// Remember that this is on resize of the container housing THIS widget, so don't update positions/sizes of the widgets
	// in the separate UndoableEditorWindow here

	setSize(containerWidth - paddingX, beginEditingButton->getPosition().y + beginEditingButton->getSize().y + paddingY);
}

void TFVGroup::setTFV(std::shared_ptr<TFV> tfv, float tfvLifespan) {
	oldTFV = tfv;
	this->tfvLifespan = tfvLifespan;
	if (dynamic_cast<PiecewiseTFV*>(tfv.get()) != nullptr) {
		this->tfv = std::dynamic_pointer_cast<PiecewiseTFV>(tfv->clone());
	} else {
		this->tfv = std::make_shared<PiecewiseTFV>();
		this->tfv->insertSegment(0, std::make_pair(0, tfv->clone()));
	}
	tfvShortDescription->setText(tfv->getName());

	//TODO: init values of the widgets in the window
}

void TFVGroup::onTFVEditorWindowResize(int windowWidth, int windowHeight) {
	panel->setSize("100%", "100%");
	panel->setPosition(0, 0);
	showGraph->setSize(100, TEXT_BUTTON_HEIGHT);
	showGraph->setPosition(paddingX, paddingY);
}

EMPAAngleOffsetGroup::EMPAAngleOffsetGroup() {
	onAngleOffsetChange = std::make_shared<entt::SigH<void(std::shared_ptr<EMPAAngleOffset>, std::shared_ptr<EMPAAngleOffset>)>>();

	offsetType = tgui::ComboBox::create();
	offsetType->setTextSize(TEXT_SIZE);
	// ID are from getID()
	offsetType->addItem("No offset", "1");
	offsetType->addItem("To player", "2");
	offsetType->addItem("To global position", "3");
	offsetType->addItem("Player sprite angle", "4");
	offsetType->connect("ItemSelected", [&](std::string item, std::string id) {
		//TODO
		onAngleOffsetChange->publish(oldAngleOffset, angleOffset);
	});
}

void EMPAAngleOffsetGroup::onContainerResize(int containerWidth, int containerHeight) {
	//TODO
}

void EMPAAngleOffsetGroup::setEMPAAngleOffset(std::shared_ptr<EMPAAngleOffset> offset) {
	oldAngleOffset = offset;
	angleOffset = offset->clone();

	//TODO: init values of widgets
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