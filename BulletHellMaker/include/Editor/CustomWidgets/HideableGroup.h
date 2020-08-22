#pragma once
#include <TGUI/TGUI.hpp>

/*
A tgui::Group whose size will be (0, 0) while invisible.
*/
class HideableGroup : public tgui::Group {
public:
	const tgui::Layout2d& getSizeLayout() const;

	void setSize(const tgui::Layout2d& size) override;
	void setSize(tgui::Layout width, tgui::Layout height);
	void setVisible(bool visible) override;

private:
	// The size of this widget right before setVisible(false) is called
	tgui::Layout2d savedSize;
};