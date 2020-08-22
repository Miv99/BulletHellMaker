#pragma once
#include <TGUI/TGUI.hpp>

/*
An edit box that accepts only numbers and can have upper/lower limits.

Signals:
	ValueChanged - emitted when the edit box value is changed, either when the enter
		key is pressed or the widget is unfocused.
		Optional parameter: the value in the edit box as a float. If this edit box is
		in integer mode, the value can just be cast to an int.
*/
class NumericalEditBoxWithLimits : public tgui::EditBox {
public:
	NumericalEditBoxWithLimits();
	static std::shared_ptr<NumericalEditBoxWithLimits> create() {
		return std::make_shared<NumericalEditBoxWithLimits>();
	}

	float getValue();
	/*
	Will not emit the ValueChanged signal.
	*/
	void setValue(int value);
	/*
	Will not emit the ValueChanged signal.
	*/
	void setValue(float value);
	/*
	Sets the min value of the edit box.
	*/
	void setMin(float min);
	/*
	Sets the max value of the edit box.
	*/
	void setMax(float max);
	/*
	Removes the min value limit on the edit box.
	*/
	void removeMin();
	/*
	Removes the max value limit on the edit box.
	*/
	void removeMax();
	/*
	Makes the edit box accept only integers.
	This is false by default.
	*/
	void setIntegerMode(bool intMode);

	inline bool getHasMin() { return hasMin; }
	inline bool getHasMax() { return hasMax; }

	tgui::Signal& getSignal(std::string signalName) override;

private:
	void updateInputValidator();

	// Emitted when the edit box value is changed
	tgui::SignalFloat onValueChange = { "ValueChanged" };

	bool hasMin = false, hasMax = false;
	float min, max;
	// if true, the inputted number must be an integer
	bool mustBeInt = false;

	float lastKnownValue;

	// bool used to ignore signals to prevent infinite loops
	bool ignoreSignals = false;
};