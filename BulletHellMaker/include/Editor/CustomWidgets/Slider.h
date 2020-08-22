#pragma once
#include <TGUI/TGUI.hpp>

/*
A tgui::Slider that allows for values not limited by multiples of the slider's step and has is slightly transparent when disabled.
*/
class Slider : public tgui::Slider {
public:
	Slider();

	void setValue(float value);
	using tgui::Slider::setValue;

	void setStep(float step);
	using tgui::Slider::setStep;
};