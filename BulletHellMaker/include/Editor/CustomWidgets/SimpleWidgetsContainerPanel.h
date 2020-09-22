#pragma once
#include <TGUI/TGUI.hpp>

/*
A tgui::ScrollablePanel designed to easily add widgets by rows/columns.

Only the width and maximum height of this widget can be set, but not the height itself.
*/
class SimpleWidgetsContainerPanel : public tgui::ScrollablePanel {
public:
	SimpleWidgetsContainerPanel();
	static std::shared_ptr<SimpleWidgetsContainerPanel> create() {
		return std::make_shared<SimpleWidgetsContainerPanel>();
	}

	/*
	Adds a widget to the bottom of the extra widgets panel.

	topPadding - vertical distance from the previously bottom-most widget in the extra widgets panel
	*/
	void addExtraRowWidget(std::shared_ptr<tgui::Widget> widget, float topPadding);
	/*
	Adds a widget to the right of the bottom-right-most widget of the extra widgets panel.
	This should only be called after at least one call to addExtraRowWidget().

	leftPadding - horizontal distance from the bottom-most widget in the extra widgets panel
	*/
	void addExtraColumnWidget(std::shared_ptr<tgui::Widget> widget, float leftPadding);

	void setWidth(tgui::Layout width);
	/*
	Sets the maximum height of this widget.
	Set width to 0 for no limit on height (this is the default).
	*/
	void setMaxHeight(tgui::Layout maxHeight);

	using tgui::ScrollablePanel::setSize;

private:
	std::shared_ptr<tgui::Widget> bottomLeftMostExtraWidget;
	std::shared_ptr<tgui::Widget> bottomRightMostExtraWidget;

	tgui::Layout width = 0;
	tgui::Layout maxHeight = 0;
};