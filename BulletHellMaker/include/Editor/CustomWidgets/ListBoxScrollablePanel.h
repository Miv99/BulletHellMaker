#pragma once
#include <TGUI/TGUI.hpp>

/*
A ScrollablePanel that can scroll horizontally as well as vertically and contains a ListBox.
The ListBox from getListBox() does not have to be added to a container, since it is already part of this ScrollablePanel.

This widget can be treated as a normal ListBox, with the exception that onListBoxItemsUpdate() should be called
anytime an item is added, removed, or changed from the ListBox.

ListBox vs ListView:
-ListBox can attach unique ID strings to items / ListView cannot
-ListBox has an ItemSelected signal / ListView does not
-ListView can select multiple items / ListBox cannot
*/
class ListBoxScrollablePanel : public tgui::ScrollablePanel {
public:
	ListBoxScrollablePanel();
	static std::shared_ptr<ListBoxScrollablePanel> create() {
		return std::make_shared<ListBoxScrollablePanel>();
	}

	/*
	Should be called anytime an item is added, removed, or changed from the ListBox.
	*/
	void onListBoxItemsUpdate();
	bool mouseWheelScrolled(float delta, tgui::Vector2f pos) override;

	void setTextSize(int textSize);
	inline std::shared_ptr<tgui::ListBox> getListBox() { return listBox; }

private:
	std::shared_ptr<tgui::ListBox> listBox;
	std::shared_ptr<tgui::Label> textWidthChecker;
	float longestWidth;
};