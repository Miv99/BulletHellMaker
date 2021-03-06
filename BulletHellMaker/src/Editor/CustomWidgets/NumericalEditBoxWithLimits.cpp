#include <Editor/CustomWidgets/NumericalEditBoxWithLimits.h>

#include <Util/StringUtils.h>

NumericalEditBoxWithLimits::NumericalEditBoxWithLimits() {
	onReturnKeyPress.connect([this](tgui::String text) {
		if (ignoreSignals) {
			return;
		}

		try {
			float value = text.toFloat();
			if (hasMax && value > max) {
				value = max;
			}
			if (hasMin && value < min) {
				value = min;
			}

			ignoreSignals = true;
			setText(formatNum(value));
			ignoreSignals = false;

			if (value != lastKnownValue) {
				lastKnownValue = value;
				onValueChange.emit(this, value);
			}
		} catch (...) {
			// Ignore exceptions
		}
	});
	onUnfocus.connect([this]() {
		if (ignoreSignals) {
			return;
		}

		try {
			float value = std::stof((std::string)getText());
			if (hasMax && value > max) {
				value = max;
			}
			if (hasMin && value < min) {
				value = min;
			}

			ignoreSignals = true;
			setText(formatNum(value));
			ignoreSignals = false;

			if (value != lastKnownValue) {
				lastKnownValue = value;
				onValueChange.emit(this, value);
			}
		} catch (...) {
			// Ignore exceptions
		}
	});
	updateInputValidator();
}

float NumericalEditBoxWithLimits::getValue() {
	if (getText() == "") return 0;
	return std::stof(std::string(getText()));
}

void NumericalEditBoxWithLimits::setValue(int value) {
	lastKnownValue = value;
	setText(formatNum(value));
}

void NumericalEditBoxWithLimits::setValue(float value) {
	lastKnownValue = value;
	if (mustBeInt) {
		setText(formatNum(std::lrint(value)));
	} else {
		setText(formatNum(value));
	}
}

void NumericalEditBoxWithLimits::setMin(float min) {
	this->min = min;
	if (getValue() < min) {
		setValue(min);
	}
	hasMin = true;
}

void NumericalEditBoxWithLimits::setMax(float max) {
	this->max = max;
	if (getValue() > max) {
		setValue(max);
	}
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

tgui::Signal& NumericalEditBoxWithLimits::getSignal(tgui::String signalName) {
	if (signalName == onValueChange.getName().toLower()) {
		return onValueChange;
	}
	return tgui::EditBox::getSignal(signalName);
}

void NumericalEditBoxWithLimits::updateInputValidator() {
	if (mustBeInt) {
		setInputValidator("^-?[0-9]*$");
	} else {
		setInputValidator("^-?[0-9]*(\.[0-9]*)?$");
	}
}