#pragma once
#include <map>

#include <TGUI/TGUI.hpp>

/*
A tgui::ListView that has some tgui::ListBox features (most importantly item IDs).

Item insertion into a specific index is not supported (because I'm lazy and there's
no real need for it yet).
*/
class ListView : public tgui::ListView {
public:
	ListView();
	static std::shared_ptr<ListView> create() {
		return std::make_shared<ListView>();
	}

	std::size_t addItem(const tgui::String& item);
	std::size_t addItem(const tgui::String& item, const tgui::String& id);

	void deselectItem();

	bool removeItem(std::size_t index);
	void removeAllItems();

	void setSelectedItemById(const tgui::String& id);
	/*
	No guarantees on which item will be chosen when there are multiple of the same one.
	*/
	void setSelectedItem(const tgui::String& item);
	void setSelectedItem(std::size_t index);

	tgui::String getSelectedItemId();
	tgui::String getItemById(const tgui::String& id);

	using tgui::ListView::insertItem;
	using tgui::ListView::insertMultipleItems;

private:
	std::map<tgui::String, tgui::String> idToItem;
	std::map<std::size_t, tgui::String> indexToId;
	std::map<tgui::String, std::size_t> idToIndex;
	std::map<tgui::String, std::size_t> itemToIndex;
};