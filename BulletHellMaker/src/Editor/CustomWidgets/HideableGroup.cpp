#include <Editor/CustomWidgets/HideableGroup.h>

const tgui::Layout2d& HideableGroup::getSizeLayout() const {
	if (!isVisible()) {
		return savedSize;
	}
	return tgui::Group::getSizeLayout();
}

void HideableGroup::setSize(const tgui::Layout2d& size) {
	// Ignore resizes until widget is set to visible again
	if (!isVisible()) {
		savedSize = size;
	} else {
		tgui::Group::setSize(size);
	}
}

void HideableGroup::setSize(tgui::Layout width, tgui::Layout height) {
	// Ignore resizes until widget is set to visible again
	if (!isVisible()) {
		savedSize = { width, height };
	} else {
		tgui::Group::setSize(width, height);
	}
}

void HideableGroup::setVisible(bool visible) {
	bool wasVisible = isVisible();

	tgui::Group::setVisible(visible);
	// Set width and height to 0 if this widget becomes invisible
	if (!visible && wasVisible) {
		savedSize = tgui::Group::getSizeLayout();
		tgui::Group::setSize({ 0, 0 });
	} else if (visible && !wasVisible) {
		tgui::Group::setSize(savedSize);
	}
}