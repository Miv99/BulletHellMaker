#include <Editor/CustomWidgets/SliderWithEditBox.h>

#include <GuiConfig.h>
#include <Editor/CustomWidgets/DelayedSlider.h>

SliderWithEditBox::SliderWithEditBox(bool useDelayedSlider) : useDelayedSlider(useDelayedSlider) {
	if (useDelayedSlider) {
		slider = std::make_shared<DelayedSlider>();
	} else {
		slider = std::make_shared<Slider>();
	}
	editBox = std::make_shared<NumericalEditBoxWithLimits>();

	editBox->setPosition(tgui::bindRight(slider) + GUI_PADDING_X, 0);

	slider->setChangeValueOnScroll(false);
	slider->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) {
			return;
		}

		ignoreSignals = true;
		editBox->setValue(value);
		ignoreSignals = false;

		if (value != lastKnownValue) {
			onValueChange.emit(this, value);
			lastKnownValue = value;
		}
	});
	editBox->connect("ValueChanged", [&](float value) {
		if (ignoreSignals) {
			return;
		}

		ignoreSignals = true;
		slider->setValue(value);
		ignoreSignals = false;

		if (value != lastKnownValue) {
			onValueChange.emit(this, value);
			lastKnownValue = value;
		}
	});

	connect("SizeChanged", [&](sf::Vector2f newSize) {
		editBox->setSize(tgui::bindMin(200, "30%"), "100%");
		// The sliding part of the slider extends upwards and downwards a bit, so make room for it
		slider->setSize(newSize.x - tgui::bindWidth(editBox) - GUI_PADDING_X - 4, newSize.y - 6);
		slider->setPosition(4, 3);
	});

	setMin(0);
	setMax(100);
	setIntegerMode(false);

	add(slider);
	add(editBox);

	lastKnownValue = slider->getValue();
}

float SliderWithEditBox::getValue() {
	return editBox->getValue();
}

void SliderWithEditBox::setValue(int value) {
	lastKnownValue = value;
	slider->setValue(value);
	editBox->setValue(value);
}

void SliderWithEditBox::setValue(float value) {
	lastKnownValue = value;
	slider->setValue(value);
	editBox->setValue(value);
}

void SliderWithEditBox::setMin(float min, bool emitValueChanged) {
	editBox->setMin(min);
	if (useDelayedSlider) {
		std::dynamic_pointer_cast<DelayedSlider>(slider)->setMinimum(min, emitValueChanged);
	} else {
		slider->setMinimum(min);
	}
}

void SliderWithEditBox::setMax(float max, bool emitValueChanged) {
	editBox->setMax(max);
	if (useDelayedSlider) {
		std::dynamic_pointer_cast<DelayedSlider>(slider)->setMaximum(max, emitValueChanged);
	} else {
		slider->setMaximum(max);
	}
}

void SliderWithEditBox::setStep(float step) {
	slider->setStep(step);
}

void SliderWithEditBox::setIntegerMode(bool intMode) {
	editBox->setIntegerMode(intMode);
	if (intMode) {
		slider->setStep(std::max((int)slider->getStep(), 1));
	}
}

void SliderWithEditBox::setTextSize(int textSize) {
	editBox->setTextSize(textSize);
}

void SliderWithEditBox::setCaretPosition(int position) {
	editBox->setCaretPosition(position);
}

tgui::Signal& SliderWithEditBox::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName())) {
		return onValueChange;
	}
	return Group::getSignal(signalName);
}