#pragma once
#include <Editor/CustomWidgets/Slider.h>

/*
A Slider that waits for some time after its value stops changing before actually emitting
the ValueChanged signal.

Signals:
	ValueChanged - emitted VALUE_CHANGE_WINDOW seconds after the slider value is changed.
		Optional parameter: the new value of the slider as a float
*/
class DelayedSlider : public Slider {
public:
	DelayedSlider();
	static std::shared_ptr<DelayedSlider> create() {
		return std::make_shared<DelayedSlider>();
	}

	bool updateTime(tgui::Duration elapsedTime) override;
	tgui::Signal& getSignal(tgui::String signalName) override;

	/*
	emitValueChanged - if the value of the slider changes as a result of changing the min/max,
		this determines whether ValueChanged signal should be emitted
	*/
	void setMinimum(float minimum, bool emitValueChanged = true);
	/*
	emitValueChanged - if the value of the slider changes as a result of changing the min/max,
		this determines whether ValueChanged signal should be emitted
	*/
	void setMaximum(float maximum, bool emitValueChanged = true);

private:
	// The amount of time that must pass since the last slider value change before
	// the onValueChange signal is emitted
	const float VALUE_CHANGE_WINDOW = 0.2f;

	// Emitted VALUE_CHANGE_WINDOW seconds after the slider value is changed
	tgui::SignalFloat onValueChange = { "ValueChanged" };
	// The amount of time that has elapsed in seconds since the last slider value change
	float timeElapsedSinceLastValueChange = 0;
	// Whether the onValueChange signal has been emitted since the last slider value change
	bool valueChangeSignalEmitted = true;

	// Used for a small hack to have getSignal() return tgui::Slider's onValueChange
	// signal rather than DelayedSlider's onValueChange when this bool is true
	bool ignoreDelayedSliderSignalName;

	bool ignoreSignals = false;
};