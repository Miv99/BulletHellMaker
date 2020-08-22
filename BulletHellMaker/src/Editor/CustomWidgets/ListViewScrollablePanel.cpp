#include <Editor/CustomWidgets/ListViewScrollablePanel.h>

ListViewScrollablePanel::ListViewScrollablePanel() {
	listView = tgui::ListView::create();
	add(listView);
	listView->setSize("100%", "100%");

	textWidthChecker = tgui::Label::create();
	textWidthChecker->setMaximumTextWidth(0);

	connect("SizeChanged", [&]() {
		onListViewItemsUpdate();
	});
}

void ListViewScrollablePanel::onListViewItemsUpdate() {
	float largestWidth = 0;
	for (auto str : listView->getItems()) {
		textWidthChecker->setText(str);
		largestWidth = std::max(largestWidth, textWidthChecker->getSize().x);
	}
	listView->setSize(std::max(getSize().x, largestWidth), "100%");
}

bool ListViewScrollablePanel::mouseWheelScrolled(float delta, tgui::Vector2f pos) {
	// This override makes it so if mouse wheel is scrolled when the scrollbar can't be scrolled any more,
	// the event is not consumed so that this widget's parent can handle the event.

	if (delta < 0) {
		// Super ghetto way of checking if scroll is already at max
		auto oldScrollbarValue = listView->getVerticalScrollbarValue();
		listView->setVerticalScrollbarValue(2000000000);
		if (listView->getVerticalScrollbarValue() == oldScrollbarValue) {
			listView->setVerticalScrollbarValue(oldScrollbarValue);
			return false;
		} else {
			listView->setVerticalScrollbarValue(oldScrollbarValue);
			return tgui::ScrollablePanel::mouseWheelScrolled(delta, pos);
		}
	} else if (listView->getVerticalScrollbarValue() == 0) {
		return false;
	} else {
		return tgui::ScrollablePanel::mouseWheelScrolled(delta, pos);
	}
}

void ListViewScrollablePanel::setTextSize(int textSize) {
	listView->setTextSize(textSize);
	textWidthChecker->setTextSize(textSize);
}