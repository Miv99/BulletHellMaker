#pragma once
#include <TGUI/TGUI.hpp>

#include <Editor/EventCapturable.h>

/*
A tgui::ChildWindow that can have a custom event handler and changes the cursor when hovering
over resize hitboxes.

This ChildWindow is resizable be default.
*/
class ChildWindow : public tgui::ChildWindow, public EventCapturable {
public:
	ChildWindow();
	~ChildWindow();
	static std::shared_ptr<ChildWindow> create() {
		return std::make_shared<ChildWindow>();
	}

	bool handleEvent(sf::Event event) override;

	/*
	Sets an event handler that handleEvent() will fallback to if an event is not directly
	handled by this widget and this widget is currently focused.
	*/
	void setFallbackEventHandler(std::function<bool(sf::Event)> handler);

private:
	std::function<bool(sf::Event)> fallbackEventHandler;

	bool draggingTop = false;
	bool draggingBottom = false;
	bool draggingLeft = false;
	bool draggingRight = false;

	bool isNearTop(const sf::Vector2f& absolutePos, const sf::Vector2f& size, const int& mouseX, const int& mouseY);
	bool isNearBottom(const sf::Vector2f& absolutePos, const sf::Vector2f& size, const int& mouseX, const int& mouseY);
	bool isNearLeft(const sf::Vector2f& absolutePos, const sf::Vector2f& size, const int& mouseX, const int& mouseY);
	bool isNearRight(const sf::Vector2f& absolutePos, const sf::Vector2f& size, const int& mouseX, const int& mouseY);
};