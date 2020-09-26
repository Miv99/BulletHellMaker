#include <Editor/CustomWidgets/EditBox.h>

EditBox::EditBox() {
	onUnfocus.connect([this]() {
		onValueChange.emit(this, this->getText());
	});
	onReturnKeyPress.connect([this](tgui::String text) {
		onValueChange.emit(this, text);
	});
}

tgui::Signal& EditBox::getSignal(tgui::String signalName) {
	if (signalName == onValueChange.getName().toLower()) {
		return onValueChange;
	}
	return tgui::EditBox::getSignal(signalName);
}