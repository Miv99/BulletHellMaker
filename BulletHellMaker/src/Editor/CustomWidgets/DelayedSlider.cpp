#include <Editor/CustomWidgets/DelayedSlider.h>

DelayedSlider::DelayedSlider() {
	// connect() will call DelayedSlider::getSignal(), so we set ignoreDelayedSliderSignalName
	// to true to get the tgui::Slider's ValueChanged signal just for this call
	ignoreDelayedSliderSignalName = true;
	connect("ValueChanged", [this]() {
		if (ignoreSignals) {
			return;
		}

		this->timeElapsedSinceLastValueChange = 0;
		this->valueChangeSignalEmitted = false;
	});
	ignoreDelayedSliderSignalName = false;
}

bool DelayedSlider::update(sf::Time elapsedTime) {
	bool ret = Slider::update(elapsedTime);

	timeElapsedSinceLastValueChange += elapsedTime.asSeconds();
	if (!valueChangeSignalEmitted && timeElapsedSinceLastValueChange >= VALUE_CHANGE_WINDOW) {
		valueChangeSignalEmitted = true;
		onValueChange.emit(this, getValue());
	}

	return ret;
}

tgui::Signal& DelayedSlider::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName()) && !ignoreDelayedSliderSignalName) {
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