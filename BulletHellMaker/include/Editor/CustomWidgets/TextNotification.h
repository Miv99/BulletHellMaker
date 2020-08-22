#pragma once
#include <TGUI/TGUI.hpp>

/*
A panel that can display text for some time before becoming invisible.
The size of this panel will scale to fit exactly the label, with some padding.
*/
class TextNotification : public tgui::Panel {
public:
	TextNotification(float notificationLifespan = 3.0f);
	static std::shared_ptr<TextNotification> create(float notificationLifespan = 3.0f) {
		return std::make_shared<TextNotification>(notificationLifespan);
	}

	bool update(sf::Time delta) override;

	void setText(std::string text);

private:
	float notificationLifespan;

	std::shared_ptr<tgui::Label> label;
	float timeUntilDisappear;
	bool textVisible = false;
};