#include <Editor/CustomWidgets/Slider.h>

#include <GuiConfig.h>

Slider::Slider() {
	getSharedRenderer()->setOpacityDisabled(WIDGET_OPACITY_DISABLED);
}

void Slider::setValue(float value) {
	// When the value is below the minimum or above the maximum then adjust it
	if (value < m_minimum)
		value = m_minimum;
	else if (value > m_maximum)
		value = m_maximum;

	if (m_value != value) {
		m_value = value;

		onValueChange.emit(this, m_value);

		updateThumbPosition();
	}
}

void Slider::setStep(float step) {
	m_step = step;
}