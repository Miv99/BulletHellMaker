#include <Editor/CustomWidgets/TabsWithPanel.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/Windows/MainEditorWindow.h>

TabsWithPanel::TabsWithPanel(EditorWindow& parentWindow) 
	: parentWindow(parentWindow) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

	tabs = tgui::Tabs::create();

	tabsContainer = tgui::ScrollablePanel::create();
	tabsContainer->setPosition(0, 0);
	add(tabsContainer);

	tabs->setPosition(0, 0);
	tabs->connect("TabSelected", &TabsWithPanel::onTabSelected, this);
	tabs->setTextSize(TEXT_SIZE);
	// Clamp tab size
	tabs->setMinimumTabWidth(TAB_WIDTH);
	tabs->setMaximumTabWidth(TAB_WIDTH);
	tabsContainer->add(tabs);
	// Make room in the height for the scrollbar
	tabsContainer->setSize("100%", tgui::bindHeight(tabs) + 30);

	// Make room for the close button for every tab
	tabNameAppendedSpaces = std::string((int)std::ceil((float)tabs->getSize().y / tabs->getTextSize()), ' ');

	moreTabsList = ListBoxScrollablePanel::create();
	moreTabsList->setTextSize(TEXT_SIZE);
	moreTabsList->getListBox()->setItemHeight(TEXT_SIZE * 1.5f);
	moreTabsList->getListBox()->connect("ItemSelected", [&](std::string tabName) {
		// Remove spaces from the end since selectTab() readds them
		selectTab(tabName.substr(0, tabName.length() - tabNameAppendedSpaces.length()));
	});

	moreTabsButton = tgui::Button::create();
	//TODO: image on this instead of a V
	moreTabsButton->setText("V");
	moreTabsButton->connect("Pressed", [&]() {
		float preferredWidth = 300;
		float preferredHeight = (moreTabsList->getListBox()->getItemHeight() + 1) * moreTabsList->getListBox()->getItemCount();
		// Set moreTabsList's position releative to the absolute position of the moreTabsButton since it will be a top-level widget
		if (moreTabsListAlignment == MoreTabsListAlignment::Left) {
			parentWindow.addPopupWidget(moreTabsList, moreTabsButton->getAbsolutePosition().x - preferredWidth + moreTabsButton->getSize().x, moreTabsButton->getAbsolutePosition().y + moreTabsButton->getSize().y, preferredWidth, preferredHeight);
		} else if (moreTabsListAlignment == MoreTabsListAlignment::Right) {
			parentWindow.addPopupWidget(moreTabsList, moreTabsButton->getAbsolutePosition().x, moreTabsButton->getAbsolutePosition().y + moreTabsButton->getSize().y, preferredWidth, preferredHeight);
		} else {
			// You forgot a case
			assert(false);
		}
	});
	add(moreTabsButton);

	connect("SizeChanged", [&](sf::Vector2f newSize) {
		moreTabsButton->setSize(tabs->getSize().y, tabs->getSize().y);
		moreTabsButton->setPosition(getSize().x - moreTabsButton->getSize().x, tgui::bindTop(tabs));

		// Update height of the tabs container
		if (tabs->getSize().x > getSize().x - moreTabsButton->getSize().x) {
			// If tabs's width > this widget's parent's width, the scrollbar is visible,
			// so make room for the scrollbar
			tabsContainer->setSize(getSize().x - moreTabsButton->getSize().x, tgui::bindHeight(tabs) + EXTRA_HEIGHT_FROM_SCROLLBAR);
		} else {
			// The scrollbar is not visible, so don't make room for the scrollbar
			tabsContainer->setSize(getSize().x - moreTabsButton->getSize().x, tgui::bindHeight(tabs));
		}
	});

	onTabsChange();
}

void TabsWithPanel::addTab(std::string tabName, std::shared_ptr<tgui::Panel> associatedPanel, bool autoSelect, bool closeable) {
	tabName += tabNameAppendedSpaces;
	if (panelsMap.count(tabName) > 0) {
		return;
	}

	panelsMap[tabName] = associatedPanel;
	associatedPanel->setPosition(0, tgui::bindBottom(tabsContainer));
	associatedPanel->setSize("100%", tgui::bindHeight(shared_from_this()) - tgui::bindBottom(tabsContainer));
	associatedPanel->setVisible(autoSelect);
	tabs->add(tabName, autoSelect);
	add(associatedPanel);

	tabsOrdering.push_back(tabName);
	if (closeable) {
		createCloseButton(tabs->getTabsCount() - 1);
	} else {
		closeButtons.push_back(std::make_pair(nullptr, ""));
	}
	onTabsChange();
}

void TabsWithPanel::insertTab(std::string tabName, std::shared_ptr<tgui::Panel> associatedPanel, int index, bool autoSelect, bool closeable) {
	tabName += tabNameAppendedSpaces;
	if (panelsMap.count(tabName) > 0) {
		return;
	}

	panelsMap[tabName] = associatedPanel;
	associatedPanel->setPosition(0, tgui::bindBottom(tabsContainer));
	associatedPanel->setSize("100%", tgui::bindHeight(shared_from_this()) - tgui::bindBottom(tabsContainer));
	associatedPanel->setVisible(autoSelect);
	tabs->insert(index, tabName, autoSelect);
	add(associatedPanel);

	tabsOrdering.insert(tabsOrdering.begin() + index, tabName);
	if (closeable) {
		createCloseButton(index);
	} else {
		closeButtons.insert(closeButtons.begin() + index, std::make_pair(nullptr, ""));
		// Reposition all close buttons that come after the created one
		for (int i = index + 1; i < tabsOrdering.size(); i++) {
			closeButtons[i].first->setPosition((i + 1) * (TAB_WIDTH + 1) - closeButtons[i].first->getSize().x, tgui::bindTop(tabs));
		}
	}
	onTabsChange();
}

void TabsWithPanel::selectTab(std::string tabName) {
	if (tabName == "") {
		return;
	}
	tabName += tabNameAppendedSpaces;

	tabs->select(tabName);
}

void TabsWithPanel::removeTab(std::string tabName) {
	tabName += tabNameAppendedSpaces;
	assert(panelsMap.count(tabName) != 0);

	remove(panelsMap[tabName]);
	if (currentPanel == panelsMap[tabName]) {
		currentPanel = nullptr;
	}
	panelsMap.erase(tabName);
	if (onSelectFunctionMap.count(tabName) > 0) {
		onSelectFunctionMap.erase(tabName);
	}
	tabs->remove(tabName);

	int pos;
	for (pos = 0; pos < tabsOrdering.size(); pos++) {
		if (tabsOrdering[pos] == tabName) break;
	}
	tabsOrdering.erase(tabsOrdering.begin() + pos);
	// Remove the tab's close button
	tabsContainer->remove(closeButtons[pos].first);
	closeButtons.erase(closeButtons.begin() + pos);
	// Reposition all close buttons that come after the removed one
	for (int i = pos; i < tabsOrdering.size(); i++) {
		closeButtons[i].first->setPosition((i + 1) * (TAB_WIDTH + 1) - closeButtons[i].first->getSize().x, tgui::bindTop(tabs));
	}

	onTabsChange();
}

void TabsWithPanel::removeAllTabs() {
	for (auto it = panelsMap.begin(); it != panelsMap.end(); it++) {
		remove(it->second);
	}
	panelsMap.clear();
	onSelectFunctionMap.clear();
	tabsOrdering.clear();
	for (std::pair<std::shared_ptr<tgui::Button>, std::string> closeButtonAndPrompt : closeButtons) {
		tabsContainer->remove(closeButtonAndPrompt.first);
	}
	closeButtons.clear();
	currentPanel = nullptr;
	tabs->removeAll();

	onTabsChange();
}

std::shared_ptr<tgui::Panel> TabsWithPanel::renameTab(std::string oldTabName, std::string newTabName) {
	std::string oldTabInternalName = oldTabName + tabNameAppendedSpaces;
	int tabIndex = -1;
	for (int i = 0; i < tabsOrdering.size(); i++) {
		if (tabsOrdering[i] == oldTabInternalName) {
			tabIndex = i;
			break;
		}
	}

	std::shared_ptr<tgui::Panel> panel = panelsMap[oldTabInternalName];
	bool closeable = (closeButtons[tabIndex].first != nullptr);
	std::string closeButtonConfirmationPrompt = closeButtons[tabIndex].second;

	bool wasSelected = tabs->getSelectedIndex() == tabIndex;
	removeTab(oldTabName);
	insertTab(newTabName, panel, tabIndex, wasSelected, closeable);
	setTabCloseButtonConfirmationPrompt(newTabName, closeButtonConfirmationPrompt);

	return panel;
}

void TabsWithPanel::setTabCloseButtonConfirmationPrompt(std::string tabName, std::string message) {
	tabName += tabNameAppendedSpaces;
	int pos;
	for (pos = 0; pos < tabsOrdering.size(); pos++) {
		if (tabsOrdering[pos] == tabName) break;
	}
	setTabCloseButtonConfirmationPrompt(pos, message);
}

void TabsWithPanel::setTabOnSelectFunction(std::string tabName, std::function<void()> function) {
	tabName += tabNameAppendedSpaces;
	onSelectFunctionMap[tabName] = function;
}

bool TabsWithPanel::hasTab(std::string tabName) {
	return panelsMap.count(tabName + tabNameAppendedSpaces) > 0;
}

int TabsWithPanel::getTabIndex(std::string tabName) {
	tabName += tabNameAppendedSpaces;
	for (int i = 0; i < tabsOrdering.size(); i++) {
		if (tabsOrdering[i] == tabName) {
			return i;
		}
	}
	return -1;
}

std::shared_ptr<tgui::Panel> TabsWithPanel::getTab(std::string name) {
	name += tabNameAppendedSpaces;
	return panelsMap.at(name);
}

void TabsWithPanel::cacheTabs(std::string tabsSetIdentifier) {
	std::vector<std::pair<std::string, std::shared_ptr<tgui::Panel>>> tabsData;
	for (auto tabName : tabsOrdering) {
		tabsData.push_back(std::make_pair(tabName, panelsMap[tabName]));
	}
	std::vector<std::string> closeButtonPrompts;
	for (std::pair<std::shared_ptr<tgui::Button>, std::string> buttonAndPrompt : closeButtons) {
		closeButtonPrompts.push_back(buttonAndPrompt.second);
	}
	tabsCache.insert(tabsSetIdentifier, { tabs->getSelected(), tabsData, closeButtonPrompts });
}

bool TabsWithPanel::isCached(std::string tabsSetIdentifier) {
	return tabsCache.contains(tabsSetIdentifier);
}

bool TabsWithPanel::loadCachedTabsSet(std::string tabsSetIdentifier) {
	if (!tabsCache.contains(tabsSetIdentifier)) {
		return false;
	}

	removeAllTabs();

	CachedTabsValue data = tabsCache.get(tabsSetIdentifier);
	for (std::pair<std::string, std::shared_ptr<tgui::Panel>> p : data.tabs) {
		addTab(p.first, p.second, p.first == data.currentlySelectedTab);
	}
	int i = 0;
	for (std::string message : data.closeButtonConfirmationPrompts) {
		setTabCloseButtonConfirmationPrompt(i, message);
		i++;
	}
	onTabsChange();
}

void TabsWithPanel::removeCachedTabsSet(std::string tabsSetIdentifier) {
	if (tabsCache.contains(tabsSetIdentifier)) {
		tabsCache.remove(tabsSetIdentifier);
	}
}

void TabsWithPanel::clearTabsCache() {
	tabsCache.clear();
}

tgui::Signal& TabsWithPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onTabClose.getName())) {
		return onTabClose;
	}
	return tgui::Group::getSignal(signalName);
}

std::string TabsWithPanel::getSelectedTab() {
	return tabs->getSelected().substring(0, tabs->getSelected().getSize() - tabNameAppendedSpaces.size());
}

void TabsWithPanel::setMoreTabsListAlignment(MoreTabsListAlignment moreTabsListAlignment) {
	this->moreTabsListAlignment = moreTabsListAlignment;
}

std::vector<std::string> TabsWithPanel::getTabNames() {
	std::vector<std::string> res;
	for (int i = 0; i < panelsMap.size(); i++) {
		std::string str = tabs->getText(i);
		res.push_back(str.substr(0, str.length() - tabNameAppendedSpaces.length()));
	}
	return res;
}

bool TabsWithPanel::handleEvent(sf::Event event) {
	// Let the currently opened panel handle the event, if it can
	if (currentPanel) {
		std::shared_ptr<EventCapturable> eventCapturer = std::dynamic_pointer_cast<EventCapturable>(currentPanel);
		if (eventCapturer) {
			return eventCapturer->handleEvent(event);
		}
	}
	return false;
}

void TabsWithPanel::onTabSelected(std::string tabName) {
	assert(panelsMap.count(tabName) != 0);

	// Hide currently open panel
	if (currentPanel) {
		currentPanel->setVisible(false);
	}

	// Show the selected tab's panel
	currentPanel = panelsMap[tabName];
	currentPanel->setVisible(true);

	moreTabsList->getListBox()->setSelectedItem(tabName);

	// Call the associated onSelect function, if any
	if (onSelectFunctionMap.count(tabName) > 0) {
		onSelectFunctionMap[tabName]();
	}
}

void TabsWithPanel::onTabsChange() {
	moreTabsList->getListBox()->removeAllItems();
	for (int i = 0; i < tabsOrdering.size(); i++) {
		moreTabsList->getListBox()->addItem(tabsOrdering[i], tabsOrdering[i]);
	}

	// More tabs button only visible if there are more than 0 tabs
	moreTabsButton->setVisible(tabsOrdering.size() > 0);

	// Update size of tabs
	tabs->setSize(tabsOrdering.size() * tabs->getMinimumTabWidth(), tabs->getSize().y);

	// Update height of the tabs container
	if (tabs->getSize().x > getSize().x - moreTabsButton->getSize().x) {
		// The scrollbar is visible, so make room for the scrollbar
		tabsContainer->setSize(getSize().x - moreTabsButton->getSize().x, tgui::bindHeight(tabs) + EXTRA_HEIGHT_FROM_SCROLLBAR);
	} else {
		// The scrollbar is not visible, so don't make room for the scrollbar
		tabsContainer->setSize(getSize().x - moreTabsButton->getSize().x, tgui::bindHeight(tabs));
	}
}

void TabsWithPanel::onCloseButtonConfirmationPromptAnswer(bool confirmed, std::string closeButtonConfirmationPromptTargetTabShortenedName) {
	if (confirmed) {
		removeTab(closeButtonConfirmationPromptTargetTabShortenedName);
		onTabClose.emit(this, closeButtonConfirmationPromptTargetTabShortenedName);
	}
}

void TabsWithPanel::setTabCloseButtonConfirmationPrompt(int index, std::string message) {
	closeButtons[index].second = message;
}

void TabsWithPanel::createCloseButton(int index) {
	// Create close button
	std::string tabName = tabs->getText(index);
	std::shared_ptr<tgui::Button> closeButton = tgui::Button::create();
	closeButton->setSize(tabs->getSize().y, tabs->getSize().y);
	closeButton->setTextSize(TEXT_SIZE);
	closeButton->setText("X");
	// +1 to the tab width because for some reason each tab's width is actually
	// 1 pixel more than TAB_WIDTH
	closeButton->setPosition((index + 1) * (TAB_WIDTH + 1) - closeButton->getSize().x, tgui::bindTop(tabs));
	closeButton->connect("Pressed", [this, tabName]() {
		int pos;
		for (pos = 0; pos < tabsOrdering.size(); pos++) {
			if (tabsOrdering[pos] == tabName) break;
		}
		std::string fullTabName = tabs->getText(pos); // Includes the spaces, so remove them
		std::string closeButtonConfirmationPromptTargetTabShortenedName = fullTabName.substr(0, fullTabName.length() - tabNameAppendedSpaces.length());
		if (closeButtons[pos].second != "") {
			parentWindow.promptConfirmation(closeButtons[pos].second, closeButtonConfirmationPromptTargetTabShortenedName, this)->sink()
				.connect<TabsWithPanel, &TabsWithPanel::onCloseButtonConfirmationPromptAnswer>(this);
		} else {
			removeTab(closeButtonConfirmationPromptTargetTabShortenedName);
		}
	});
	closeButtons.insert(closeButtons.begin() + index, std::make_pair(closeButton, ""));
	// Reposition all close buttons that come after the created one
	for (int i = index + 1; i < tabsOrdering.size(); i++) {
		closeButtons[i].first->setPosition((i + 1) * (TAB_WIDTH + 1) - closeButtons[i].first->getSize().x, tgui::bindTop(tabs));
	}
	tabsContainer->add(closeButton);
}