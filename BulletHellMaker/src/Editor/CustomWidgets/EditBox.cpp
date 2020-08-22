#include <Editor/CustomWidgets/EditBox.h>

EditBox::EditBox() {
	connect("Unfocused", [this]() {
		onValueChange.emit(this, this->getText());
	});
	connect("ReturnKeyPressed", [this](std::string text) {
		onValueChange.emit(this, text);
	});
}

tgui::Signal& EditBox::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onValueChange.getName())) {
		return onValueChange;
	}
	return tgui::EditBox::getSignal(signalName);
}