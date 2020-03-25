#pragma once
#include <TGUI/TGUI.hpp>

/*
Signifies that the object that inherits this class is able to capture
GUI events.

Note that you'll have to call handleEvent() yourself.
*/
class EventCapturable {
public:
	/*
	Returns whether the event should be consumed by this object.
	*/
	virtual bool handleEvent(sf::Event event) {
		return false;
	}
};