#include <Editor/CustomWidgets/TimedLabel.h>

TimedLabel::TimedLabel(float charDelay) : charDelay(charDelay) {
}

bool TimedLabel::update(sf::Time elapsedTime) {
	timeSinceTextSet += elapsedTime.asSeconds();
	if (timeSinceTextSet > initialDelay) {
		timeSinceLastChar += elapsedTime.asSeconds();
		int numCharsToBeShown = (int)(timeSinceLastChar / charDelay);
		if (numCharsToBeShown > 0) {
			numVisibleChars = std::min(numVisibleChars + numCharsToBeShown, textNumChars);
			Label::setText(text.substr(0, numVisibleChars));
		}
		timeSinceLastChar = std::fmod(timeSinceLastChar, charDelay);
	}
	return Label::update(elapsedTime);
}

void TimedLabel::setText(const sf::String& text, float initialDelay) {
	this->text = text;
	this->initialDelay = initialDelay;
	textNumChars = text.getSize();
	timeSinceTextSet = 0;
	timeSinceLastChar = 0;
	numVisibleChars = 0;

	Label::setText("");
}