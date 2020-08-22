#include <Editor/CustomWidgets/TextNotification.h>

#include <GuiConfig.h>

TextNotification::TextNotification(float notificationLifespan) : notificationLifespan(notificationLifespan) {
	label = tgui::Label::create();
	label->setTextSize(TEXT_SIZE);
	label->setPosition(GUI_PADDING_X, GUI_LABEL_PADDING_Y);
	add(label);

	setSize(tgui::bindWidth(label) + GUI_PADDING_X * 2, tgui::bindHeight(label) + GUI_LABEL_PADDING_Y * 2);
}

bool TextNotification::update(sf::Time delta) {
	if (textVisible) {
		timeUntilDisappear -= delta.asSeconds();
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