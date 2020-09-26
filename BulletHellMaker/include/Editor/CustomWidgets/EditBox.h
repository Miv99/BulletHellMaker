#pragma once
#include <TGUI/TGUI.hpp>

/*
An EditBox with random miscellaneous improvements.

Signals:
	ValueChanged - emitted when the edit box text is changed, either when the enter
		key is pressed or the widget is unfocused.
		Optional parameter: the text in the edit box as a string
*/
class EditBox : public tgui::EditBox {
public:
	// Emitted when return key is pressed or when the widget is unfocused
	tgui::SignalString onValueChange = { "ValueChanged" };

	EditBox();
	static std::shared_ptr<EditBox> create() {
		return std::make_shared<EditBox>();
	}

	tgui::Signal& getSignal(tgui::String signalName) override;

private:
};