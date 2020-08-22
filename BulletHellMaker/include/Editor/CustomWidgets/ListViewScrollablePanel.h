#pragma once
#include <TGUI/TGUI.hpp>

/*
A ScrollablePanel that can scroll horizontally as well as vertically and contains a ListView.
The ListView from getListView() does not have to be added to a container, since it is already part of this ScrollablePanel.

This widget can be treated as a normal ListView, with the exception that onListViewItemsUpdate() should be called
anytime an item is added, removed, or changed from the ListView.

ListBox vs ListView:
-ListBox can attach unique ID strings to items / ListView cannot
-ListBox has an ItemSelected signal / ListView does not
-ListView can select multiple items / ListBox cannot
*/
class ListViewScrollablePanel : public tgui::ScrollablePanel {
public:
	ListViewScrollablePanel();
	static std::shared_ptr<ListViewScrollablePanel> create() {
		return std::make_shared<ListViewScrollablePanel>();
	}

	/*
	Should be called anytime an item is added, removed, or has its text changed from the ListView.
	*/
	void onListViewItemsUpdate();
	bool mouseWheelScrolled(float delta, tgui::Vector2f pos) override;

	void setTextSize(int textSize);
	inline std::shared_ptr<tgui::ListView> getListView() { return listView; }

protected:
	std::shared_ptr<tgui::ListView> listView;

private:
	std::shared_ptr<tgui::Label> textWidthChecker;
	float longestWidth;
};