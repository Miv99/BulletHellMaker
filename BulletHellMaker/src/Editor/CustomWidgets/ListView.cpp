#include <Editor/CustomWidgets/ListView.h>

ListView::ListView() {
}

std::size_t ListView::addItem(const tgui::String& item) {
	std::size_t index = tgui::ListView::addItem(item);
	itemToIndex[item] = index;
	return index;
}

std::size_t ListView::addItem(const tgui::String& item, const tgui::String& id) {
	std::size_t index = tgui::ListView::addItem(item);
	idToItem[id] = item;
	indexToId[index] = id;
	idToIndex[id] = index;
	itemToIndex[item] = index;
	return index;
}

void ListView::deselectItem() {
	tgui::ListView::deselectItems();
}

bool ListView::removeItem(std::size_t index) {
	if (indexToId.find(index) != indexToId.end()) {
		const tgui::String& id = indexToId[index];
		if (idToItem.find(id) != idToItem.end()) {
			idToItem.erase(id);
		}
		if (idToIndex.find(id) != idToIndex.end()) {
			idToIndex.erase(id);
		}
		indexToId.erase(index);
	}
	return tgui::ListView::removeItem(index);
}

void ListView::removeAllItems() {
	tgui::ListView::removeAllItems();
	idToItem.clear();
	indexToId.clear();
	idToIndex.clear();
	itemToIndex.clear();
}

void ListView::setSelectedItemById(const tgui::String& id) {
	if (idToIndex.find(id) != idToIndex.end()) {
		tgui::ListView::setSelectedItem(idToIndex[id]);
	}
}

void ListView::setSelectedItem(const tgui::String& item) {
	if (itemToIndex.find(item) != itemToIndex.end()) {
		tgui::ListView::setSelectedItem(itemToIndex[item]);
	}
}

void ListView::setSelectedItem(std::size_t index) {
	tgui::ListView::setSelectedItem(index);
}

tgui::String ListView::getSelectedItemId() {
	std::size_t index = tgui::ListView::getSelectedItemIndex();
	if (indexToId.find(index) != indexToId.end()) {
		return indexToId[index];
	}
	return tgui::String();
}

tgui::String ListView::getItemById(const tgui::String& id) {
	if (idToItem.find(id) != idToItem.end()) {
		return idToItem[id];
	}
	return tgui::String();
}
