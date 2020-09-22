#include <Editor/CustomWidgets/SimpleWidgetsContainerPanel.h>

#include <GuiConfig.h>

SimpleWidgetsContainerPanel::SimpleWidgetsContainerPanel() 
	: tgui::ScrollablePanel() {
}

void SimpleWidgetsContainerPanel::addExtraRowWidget(std::shared_ptr<tgui::Widget> widget, float topPadding) {
	if (bottomLeftMostExtraWidget) {
		widget->setPosition(GUI_PADDING_X, tgui::bindBottom(bottomLeftMostExtraWidget) + topPadding);
	} else {
		widget->setPosition(GUI_PADDING_X, topPadding);
	}
	setSize(width, tgui::bindBottom(widget) + GUI_PADDING_Y);
	add(widget);
	bottomLeftMostExtraWidget = widget;
	bottomRightMostExtraWidget = widget;
}

void SimpleWidgetsContainerPanel::addExtraColumnWidget(std::shared_ptr<tgui::Widget> widget, float leftPadding) {
	assert(bottomRightMostExtraWidget != nullptr);
	widget->setPosition(tgui::bindRight(bottomRightMostExtraWidget) + leftPadding, tgui::bindTop(bottomRightMostExtraWidget));
	add(widget);
	bottomRightMostExtraWidget = widget;
}

void SimpleWidgetsContainerPanel::setWidth(tgui::Layout width) {
	this->width = width;
	if (bottomRightMostExtraWidget) {
		if (maxHeight.isConstant() && maxHeight.getValue() == 0) {
			setSize(width, tgui::bindBottom(bottomRightMostExtraWidget) + GUI_PADDING_Y);
		} else {
			setSize(width, tgui::bindMin(maxHeight, tgui::bindBottom(bottomRightMostExtraWidget) + GUI_PADDING_Y));
		}
	} else {
		setSize(width, 0);
	}
}

void SimpleWidgetsContainerPanel::setMaxHeight(tgui::Layout maxHeight) {
	this->maxHeight = maxHeight;
	if (bottomRightMostExtraWidget) {
		if (maxHeight.isConstant() && maxHeight.getValue() == 0) {
			setSize(width, tgui::bindBottom(bottomRightMostExtraWidget) + GUI_PADDING_Y);
		} else {
			setSize(width, tgui::bindMin(maxHeight, tgui::bindBottom(bottomRightMostExtraWidget) + GUI_PADDING_Y));
		}
	} else {
		setSize(width, 0);
	}
}
