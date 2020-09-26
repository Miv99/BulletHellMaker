#pragma once
#include <mutex>
#include <Editor/CustomWidgets/HideableGroup.h>
#include <Editor/EventCapturable.h>
#include <Editor/CopyPaste.h>
#include <Editor/CustomWidgets/ListView.h>
#include <Editor/CustomWidgets/NumericalEditBoxWithLimits.h>
#include <Editor/CustomWidgets/SliderWithEditBox.h>
#include <Editor/Util/ExtraSignals.h>

class EditorWindow;

/*
Used to edit TFVs.

Signals:
	ValueChanged - emitted when a change is made to the TFV being edited.
		Optional parameter: a pair of a shared_ptr to the old TFV object
			and shared_ptr to the new TFV object
*/
class TFVGroup : public HideableGroup, public EventCapturable, public CopyPasteable {
public:
	/*
	Signal emitted when a change is made to the TFV being edited.
	Optional parameter: a pair of a shared_ptr to the old TFV object
		and shared_ptr to the new TFV object
	*/
	tgui::SignalTwoTFVs onValueChange = { "ValueChanged" };

	TFVGroup(EditorWindow& parentWindow, Clipboard& clipboard);
	static std::shared_ptr<TFVGroup> create(EditorWindow& parentWindow, Clipboard& clipboard) {
		return std::make_shared<TFVGroup>(parentWindow, clipboard);
	}

	CopyOperationResult copyFrom() override;
	PasteOperationResult pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	PasteOperationResult paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	/*
	Initialize this widget to tfv's values.
	tfv won't be modified by this widget.
	The old TFV emitted in the ValueChanged signal will tfv until the next setTFV() call.

	tfvLifespan - the max lifespan of tfv. If the TFV is in an EMPA, this value
		should be EMPA::getTime().
	*/
	void setTFV(std::shared_ptr<TFV> tfv, float tfvLifespan);

	tgui::Signal& getSignal(tgui::String signalName) override;

private:
	// Time between each tfv curve vertex
	const static float TFV_TIME_RESOLUTION;

	EditorWindow& parentWindow;
	Clipboard& clipboard;
	std::recursive_mutex tfvMutex;

	std::shared_ptr<tgui::Button> showGraph;

	std::shared_ptr<tgui::Button> addSegment;
	std::shared_ptr<tgui::Button> deleteSegment;
	std::shared_ptr<tgui::Label> startTimeLabel;
	std::shared_ptr<SliderWithEditBox> startTime;
	std::shared_ptr<tgui::Button> changeSegmentType;
	std::shared_ptr<tgui::ListBox> segmentTypePopup;

	std::shared_ptr<ListView> segmentList; // Each item ID is the index of the segment in tfv's segment vector
	std::shared_ptr<tgui::Label> tfvFloat1Label;
	std::shared_ptr<NumericalEditBoxWithLimits> tfvFloat1EditBox;
	std::shared_ptr<tgui::Label> tfvFloat2Label;
	std::shared_ptr<NumericalEditBoxWithLimits> tfvFloat2EditBox;
	std::shared_ptr<tgui::Label> tfvFloat3Label;
	std::shared_ptr<NumericalEditBoxWithLimits> tfvFloat3EditBox;
	std::shared_ptr<tgui::Label> tfvFloat4Label;
	std::shared_ptr<NumericalEditBoxWithLimits> tfvFloat4EditBox;
	std::shared_ptr<tgui::Label> tfvInt1Label;
	std::shared_ptr<NumericalEditBoxWithLimits> tfvInt1EditBox;

	std::shared_ptr<TFV> oldTFV; // Should never be modified after setTFV() is called
	std::shared_ptr<PiecewiseTFV> tfv;
	std::shared_ptr<TFV> selectedSegment;
	int selectedSegmentIndex = -1;

	float tfvLifespan; // Shouldn't be modified except by setTFV()

	// bool used to ignore signals to prevent infinite loops
	bool ignoreSignals = false;
	bool ignoreResizeSignal = false;

	void deselectSegment();
	void selectSegment(int index);
	void populateSegmentList();
};