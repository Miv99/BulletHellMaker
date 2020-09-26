#pragma once
#include <TGUI/TGUI.hpp>

/*
A tgui::Label that shows text over time rather than all at once.
*/
class TimedLabel : public tgui::Label {
public:
	/*
	charDelay - time in seconds before the next character is shown
	*/
	TimedLabel(float charDelay = 1 / 9.0f);
	static std::shared_ptr<TimedLabel> create(float charDelay = 1 / 9.0f) {
		return std::make_shared<TimedLabel>(charDelay);
	}

	bool updateTime(tgui::Duration elapsedTime) override;
	/*
	initialDelay -  time in seconds before the first character is shown
	*/
	void setText(const sf::String& text, float initialDelay = 0);

private:
	// The full text to be shown
	std::string text;
	// Number of characters in text
	int textNumChars = 0;
	// Current number of visible characters
	int numVisibleChars = 0;
	// Time in seconds since the last char was shown
	float timeSinceLastChar = 0;
	// Time in seconds since the last setText() call
	float timeSinceTextSet = 0;
	// Time in seconds before the next character is shown
	float charDelay;
	// Time in seconds before the first character is shown
	float initialDelay;
};