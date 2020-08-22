#pragma once
#include <Editor/CustomWidgets/HideableGroup.h>
#include <Editor/CustomWidgets/Slider.h>
#include <Editor/CustomWidgets/NumericalEditBoxWithLimits.h>

/*
A Group containing a slider whose value can be set with a
NumericalEditBoxWithLimits located on its right.

Signals:
	ValueChanged - emitted VALUE_CHANGE_WINDOW seconds after the slider value or edit box value is changed.
			Optional parameter: the new value as a float
*/
class SliderWithEditBox : public HideableGroup {
public:
	/*
	useDelayedSlider - if true, a DelayedSlider will be used instead of a Slider
	*/
	SliderWithEditBox(bool useDelayedSlider = true);
	static std::shared_ptr<SliderWithEditBox> create(bool useDelayedSlider = true) {
		return std::make_shared<SliderWithEditBox>(useDelayedSlider);
	}

	float getValue();

	/*
	Sets the value of the slider and edit box without emitting the ValueChanged signal.
	*/
	void setValue(int value);
	/*
	Sets the value of the slider and edit box without emitting the ValueChanged signal.
	*/
	void setValue(float value);
	/*
	Sets the min value of the slider and edit box.

	emitValueChanged - if the value of the slider changes as a result of changing the min/max,
		this determines whether ValueChanged signal should be emitted. This value is treated as
		being always true if this widget was constructed with useDelayedSlider false.
	*/
	void setMin(float min, bool emitValueChanged = true);
	/*
	Sets the max value of the slider and edit box.

	emitValueChanged - if the value of the slider changes as a result of changing the min/max,
		this determines whether ValueChanged signal should be emitted. This value is treated as
		being always true if this widget was constructed with useDelayedSlider false.
	*/
	void setMax(float max, bool emitValueChanged = true);
	/*
	Sets the step of the slider.
	*/
	void setStep(float step);
	/*
	Whether the slider and edit box can accept only integers.
	*/
	void setIntegerMode(bool intMode);
	/*
	Sets the text size of the edit box.
	*/
	void setTextSize(int textSize);
	void setCaretPosition(int position);

	tgui::Signal& getSignal(std::string signalName) override;

private:
	bool useDelayedSlider;
	std::shared_ptr<Slider> slider;
	std::shared_ptr<NumericalEditBoxWithLimits> editBox;

	tgui::SignalFloat onValueChange = { "ValueChanged" };

	float lastKnownValue;

	// bool used to ignore signals to prevent infinite loops
	bool ignoreSignals = false;
};