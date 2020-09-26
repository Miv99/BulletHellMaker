#include <Editor/CustomWidgets/TextNotification.h>

#include <Mutex.h>
#include <GuiConfig.h>

TextNotification::TextNotification(float notificationLifespan)
	: notificationLifespan(notificationLifespan) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	label = tgui::Label::create();
	label->setTextSize(TEXT_SIZE);
	label->setPosition(GUI_PADDING_X, GUI_LABEL_PADDING_Y);
	add(label);

	setSize(tgui::bindWidth(label) + GUI_PADDING_X * 2, tgui::bindHeight(label) + GUI_LABEL_PADDING_Y * 2);
}

bool TextNotification::updateTime(tgui::Duration elapsedTime) {
	if (textVisible) {
		timeUntilDisappear -= elapsedTime.asSeconds();
		if (timeUntilDisappear <= 0) {
			setVisible(false);
			textVisible = false;
			return true;
		}
	}
	return false;
}

void TextNotification::setText(std::string text) {
	label->setText(text);
	textVisible = true;
	timeUntilDisappear = notificationLifespan;
	setVisible(true);
}