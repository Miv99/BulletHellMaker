#include <Editor/CustomWidgets/DelayedSlider.h>

DelayedSlider::DelayedSlider() {
	// connect() will call DelayedSlider::getSignal(), so we set ignoreDelayedSliderSignalName
	// to true to get the tgui::Slider's ValueChanged signal just for this call
	ignoreDelayedSliderSignalName = true;
	onValueChange.connect([this]() {
		if (ignoreSignals) {
			return;
		}

		this->timeElapsedSinceLastValueChange = 0;
		this->valueChangeSignalEmitted = false;
	});
	ignoreDelayedSliderSignalName = false;
}

bool DelayedSlider::updateTime(tgui::Duration elapsedTime) {
	bool ret = Slider::updateTime(elapsedTime);

	timeElapsedSinceLastValueChange += elapsedTime.asSeconds();
	if (!valueChangeSignalEmitted && timeElapsedSinceLastValueChange >= VALUE_CHANGE_WINDOW) {
		valueChangeSignalEmitted = true;
		onValueChange.emit(this, getValue());
	}

	return ret;
}

tgui::Signal& DelayedSlider::getSignal(tgui::String signalName) {
	if (signalName == onValueChange.getName().toLower() && !ignoreDelayedSliderSignalName) {
		return onValueChange;
	}
	return Slider::getSignal(signalName);
}

void DelayedSlider::setMinimum(float minimum, bool emitValueChanged) {
	ignoreSignals = !emitValueChanged;
	Slider::setMinimum(minimum);
	ignoreSignals = false;
}

void DelayedSlider::setMaximum(float maximum, bool emitValueChanged) {
	ignoreSignals = !emitValueChanged;
	Slider::setMaximum(maximum);
	ignoreSignals = false;
}