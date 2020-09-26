#pragma once
#include <Editor/CustomWidgets/HideableGroup.h>
#include <Editor/CustomWidgets/SliderWithEditBox.h>
#include <Editor/Util/ExtraSignals.h>
#include <Game/AudioPlayer.h>

/*
Used to edit a SoundSettings object.
IMPORTANT: there's some bug where if this widget's width is bound to something (with tgui::bind____), resizing it will
cause a crash. So it has to be updated whenever whatever widget you want to bind this widget's width to emits its SizeChanged signal.

Signals:
	ValueChanged - emitted when a change is made to the SoundSettings object being edited
	Optional parameter: the new SoundSettings object
*/
class SoundSettingsGroup : public HideableGroup {
public:
	// Emitted when a change is made to the SoundSettings object being edited
	tgui::SignalSoundSettings onValueChange = { "ValueChanged" };

	SoundSettingsGroup(std::string pathToSoundsFolder);
	static std::shared_ptr<SoundSettingsGroup> create(std::string pathToSoundsFolder) {
		return std::make_shared<SoundSettingsGroup>(pathToSoundsFolder);
	}

	/*
	Initialize the widgets in this group to some SoundSettings's values
	*/
	void initSettings(SoundSettings init);
	/*
	Populate the file names list with all supported sound files in the directory
	*/
	void populateFileNames(std::string pathToSoundsFolder);

	void setEnabled(bool enabled);

	tgui::Signal& getSignal(tgui::String signalName) override;

private:
	std::shared_ptr<tgui::CheckBox> enableAudio;
	std::shared_ptr<tgui::ComboBox> fileName;
	std::shared_ptr<SliderWithEditBox> volume;
	std::shared_ptr<SliderWithEditBox> pitch;

	std::shared_ptr<tgui::Label> fileNameLabel;
	std::shared_ptr<tgui::Label> volumeLabel;
	std::shared_ptr<tgui::Label> pitchLabel;

	// bool used to ignore signals to prevent infinite loops
	bool ignoreSignals = false;
	bool ignoreResizeSignal = false;
};