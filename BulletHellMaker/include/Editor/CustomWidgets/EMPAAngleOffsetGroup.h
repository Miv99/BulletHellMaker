#pragma once
#include <Editor/CustomWidgets/HideableGroup.h>
#include <Editor/CustomWidgets/EditBox.h>
#include <Editor/Util/ExtraSignals.h>
#include <LevelPack/EditorMovablePointAction.h>

class EditorWindow;

/*
Used to edit EMPAAngleOffsets.

Signals:
	ValueChanged - emitted when a change is made to the EMPAAngleOffset being edited.
		Optional parameter: a pair of a shared_ptr to the old EMPAAngleOffset object
			and shared_ptr to the new EMPAAngleOffset object
*/
class EMPAAngleOffsetGroup : public HideableGroup {
public:
	EMPAAngleOffsetGroup(EditorWindow& parentWindow);
	static std::shared_ptr<EMPAAngleOffsetGroup> create(EditorWindow& parentWindow) {
		return std::make_shared<EMPAAngleOffsetGroup>(parentWindow);
	}

	/*
	Initialize this widget to offset's values.
	offset won't be modified by this widget.

	The old offset emitted in the ValueChanged signal will offset until the next setEMPAAngleOffset() call.
	*/
	void setEMPAAngleOffset(std::shared_ptr<EMPAAngleOffset> offset);

	tgui::Signal& getSignal(std::string signalName) override;

private:
	EditorWindow& parentWindow;

	std::shared_ptr<tgui::Label> offsetName;
	std::shared_ptr<tgui::Button> changeType;
	std::shared_ptr<tgui::ListBox> typePopup;
	std::shared_ptr<tgui::Label> xLabel;
	std::shared_ptr<tgui::Label> yLabel;
	std::shared_ptr<EditBox> x;
	std::shared_ptr<EditBox> y;

	std::shared_ptr<EMPAAngleOffset> oldOffset; // Should never be modified after setTFV() is called
	std::shared_ptr<EMPAAngleOffset> offset;

	/*
	Signal emitted when a change is made to the EMPAAngleOffset being edited.
	Optional parameter: a pair of a shared_ptr to the old EMPAAngleOffset object
		and shared_ptr to the new EMPAAngleOffset object
	*/
	tgui::SignalEMPAAngleOffsetPair onValueChange = { "ValueChanged" };

	// bool used to ignore signals to prevent infinite loops
	bool ignoreSignals = false;
	bool ignoreResizeSignal = false;

	void updateWidgets();
};