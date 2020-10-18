#include <Editor/CustomWidgets/SoundSettingsGroup.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/Util/EditorUtils.h>

SoundSettingsGroup::SoundSettingsGroup(std::string pathToSoundsFolder) {
	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	enableAudio = tgui::CheckBox::create();
	fileName = tgui::ComboBox::create();
	volume = SliderWithEditBox::create();
	pitch = SliderWithEditBox::create();
	fileNameLabel = tgui::Label::create();
	volumeLabel = tgui::Label::create();
	pitchLabel = tgui::Label::create();
	//TODO: add a play/stop button to test sounds and a progressbar to see the length of the selected audio

	enableAudio->setToolTip(createToolTip("This sound will be played only if this is checked."));
	fileNameLabel->setToolTip(createToolTip("The name of the audio file. Only WAV, OGG/Vorbis, and FLAC files are supported. Files must be in the folder \"" + pathToSoundsFolder + "\""));
	volumeLabel->setToolTip(createToolTip("The volume of the audio when it is played."));
	pitchLabel->setToolTip(createToolTip("The pitch of the audio when it is played."));

	volume->setMin(0, false);
	volume->setMax(100, false);
	volume->setStep(1);
	pitch->setMin(1, false);
	pitch->setMax(10, false);
	pitch->setStep(0.01f);

	enableAudio->setTextSize(TEXT_SIZE);
	fileNameLabel->setTextSize(TEXT_SIZE);
	volumeLabel->setTextSize(TEXT_SIZE);
	pitchLabel->setTextSize(TEXT_SIZE);

	enableAudio->setText("Enable sound");
	fileNameLabel->setText("Sound file");
	volumeLabel->setText("Volume");
	pitchLabel->setText("Pitch");

	populateFileNames(pathToSoundsFolder);

	enableAudio->onChange.connect([this]() {
		if (ignoreSignals) {
			return;
		}

		onValueChange.emit(this, SoundSettings(static_cast<std::string>(fileName->getSelectedItem()), volume->getValue(), pitch->getValue(), !enableAudio->isChecked()));
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
	fileName->onItemSelect.connect([this]() {
		if (ignoreSignals) {
			return;
		}

		onValueChange.emit(this, SoundSettings((std::string)fileName->getSelectedItem(), volume->getValue(), pitch->getValue(), !enableAudio->isChecked()));
	});
	volume->onValueChange.connect([this](float value) {
		if (ignoreSignals) {
			return;
		}

		onValueChange.emit(this, SoundSettings((std::string)fileName->getSelectedItem(), volume->getValue(), pitch->getValue(), !enableAudio->isChecked()));
	});
	pitch->onValueChange.connect([this](float value) {
		if (ignoreSignals) {
			return;
		}

		onValueChange.emit(this, SoundSettings((std::string)fileName->getSelectedItem(), volume->getValue(), pitch->getValue(), !enableAudio->isChecked()));
	});

	onSizeChange.connect([this](sf::Vector2f newSize) {
		if (ignoreResizeSignal) {
			return;
		}

		ignoreResizeSignal = true;

		enableAudio->setSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
		fileName->setSize(newSize.x, TEXT_BOX_HEIGHT);
		volume->setSize(newSize.x, SLIDER_HEIGHT);
		pitch->setSize(newSize.x, SLIDER_HEIGHT);

		enableAudio->setPosition(0, 0);
		fileNameLabel->setPosition(tgui::bindLeft(enableAudio), enableAudio->getPosition().y + enableAudio->getSize().y + GUI_PADDING_Y);
		fileName->setPosition(tgui::bindLeft(enableAudio), fileNameLabel->getPosition().y + fileNameLabel->getSize().y + GUI_LABEL_PADDING_Y);
		volumeLabel->setPosition(tgui::bindLeft(enableAudio), fileName->getPosition().y + fileName->getSize().y + GUI_PADDING_Y);
		volume->setPosition(tgui::bindLeft(enableAudio), volumeLabel->getPosition().y + volumeLabel->getSize().y + GUI_LABEL_PADDING_Y);
		pitchLabel->setPosition(tgui::bindLeft(enableAudio), volume->getPosition().y + volume->getSize().y + GUI_PADDING_Y);
		pitch->setPosition(tgui::bindLeft(enableAudio), pitchLabel->getPosition().y + pitchLabel->getSize().y + GUI_LABEL_PADDING_Y);
		this->setSize(this->getSizeLayout().x, pitch->getPosition().y + pitch->getSize().y + GUI_PADDING_Y);
		ignoreResizeSignal = false;
	});

	add(enableAudio);
	add(fileName);
	add(volume);
	add(pitch);
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

	fileNameLabel->setToolTip(createToolTip("The name of the audio file. Only WAV, OGG/Vorbis, and FLAC files are supported. Files must be in the folder \"" + pathToSoundsFolder + "\""));

	// Populate fileName with all supported sound files in the directory
	for (const auto& entry : std::filesystem::directory_iterator(pathToSoundsFolder)) {
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

tgui::Signal& SoundSettingsGroup::getSignal(tgui::String signalName) {
	if (signalName == onValueChange.getName().toLower()) {
		return onValueChange;
	}
	return Group::getSignal(signalName);
}