#pragma once
#include <TGUI/TGUI.hpp>

#include <DataStructs/LRUCache.h>
#include <Editor/EventCapturable.h>
#include <Editor/CustomWidgets/ListBoxScrollablePanel.h>

class EditorWindow;

/*
A widget with tabs that, when one is selected, shows a Panel associated with that tab.
Warning: undefined behavior when there are multiple tabs with the same name, or when
a tab has an empty string for a name.

This widget will automatically call the selected tab's handleEvent() when this
widget's handleEvent() is called.

This widget is intended to be used for a small number of tabs. It might get
laggy when there's a lot.
*/
class TabsWithPanel : public tgui::Group, public EventCapturable {
public:
	enum MoreTabsListAlignment {
		Left, // The right side of moreTabsList will be x-aligned with the right side of moreTabsButton
		Right // The left side of moreTabsList will be x-aligned with the left side of moreTabsButton.
	};

	/*
	parentWindow - the EditorWindow that this widget will be or has been added to
	*/
	TabsWithPanel(EditorWindow& parentWindow);
	inline static std::shared_ptr<TabsWithPanel> create(EditorWindow& parentWindow) {
		return std::make_shared<TabsWithPanel>(parentWindow);
	}

	/*
	tabName - the unique name of the new tab
	associatedPanel - the Panel that will be shown when the tab is selected
	autoSelect - whether to select the new tab immediately
	closeable - whether to have a button that can close this tab
	*/
	void addTab(std::string tabName, std::shared_ptr<tgui::Panel> associatedPanel, bool autoSelect = true, bool closeable = false);
	/*
	tabName - the unique name of the new tab
	associatedPanel - the Panel that will be shown when the tab is selected
	index - the new index of the tab
	autoSelect - whether to select the new tab immediately
	closeable - whether to have a button that can close this tab
	*/
	void insertTab(std::string tabName, std::shared_ptr<tgui::Panel> associatedPanel, int index, bool autoSelect = true, bool closeable = false);
	/*
	Selects a tab.
	tabName - the unique name of the tab
	*/
	void selectTab(std::string tabName);
	/*
	Removes the tab.
	tabName - the unique name of the tab
	*/
	void removeTab(std::string tabName);
	/*
	Removes all tabs.
	*/
	void removeAllTabs();
	/*
	Rename a tab.
	Undefined behavior if newTabName is already an existing tab name or if oldTabName does not exist.
	Returns the panel associated with the renamed tab.
	*/
	std::shared_ptr<tgui::Panel> renameTab(std::string oldTabName, std::string newTabName);
	/*
	Mark/Unmark a specific tab as requiring pressing "Yes" to a confirmation prompt before
	the tab is actually closed. This function does nothing if hasCloseButtons is false.
	By default, no tab requires a confirmation prompt.

	tabName - the unique name of the tab
	message - the string to be displayed in the confirmation prompt.
		Set to an empty string to disable the confirmation prompt.
	*/
	void setTabCloseButtonConfirmationPrompt(std::string tabName, std::string message);
	/*
	Sets a function to be called whenever a certain tab is selected.
	*/
	void setTabOnSelectFunction(std::string tabName, std::function<void()> function);
	/*
	Returns whether the tab exists.
	tabName - the unique name of the tab
	*/
	bool hasTab(std::string tabName);
	/*
	Returns the index of some tab, or -1 if the tab with the given tab name does
	not exist.
	tabName - the unique name of the tab
	*/
	int getTabIndex(std::string tabName);
	/*
	Returns the panel for some tab name.
	*/
	std::shared_ptr<tgui::Panel> getTab(std::string name);

	/*
	Caches all currently added tabs so that the current set of tabs can be reopened at a later time.
	If tabsSetIdentifier already refers to some set of tabs, the old set of tabs will be overriden.

	tabsSetIdentifier - the unique identifier that will later be used to reopen the cached tabs
	*/
	void cacheTabs(std::string tabsSetIdentifier);
	/*
	Returns whether tabsSetIdentifier refers to any set of cached tabs.

	See cacheTabs() for more info on tabsSetIdentifier.
	*/
	bool isCached(std::string tabsSetIdentifier);
	/*
	Removes all current tabs and reopens the set of tabs referred to by tabsSetIdentifier.
	If tabsSetIdentifier does not refer to any set of tabs, this function will do nothing and return false.
	Otherwise, if it returns true.

	See cacheTabs() for more info on tabsSetIdentifier.
	*/
	bool loadCachedTabsSet(std::string tabsSetIdentifier);
	/*
	Removes the set of tabs that tabsSetIdentifier refers to from the cache.
	If tabsSetIdentifier does not refer to any set of tabs, this function will do nothing.

	See cacheTabs() for more info on tabsSetIdentifier.
	*/
	void removeCachedTabsSet(std::string tabsSetIdentifier);
	/*
	Clears the tabs cache.
	*/
	void clearTabsCache();

	tgui::Signal& getSignal(std::string signalName) override;
	/*
	Returns the unique name of the currently selected tab.
	*/
	std::string getSelectedTab();
	/*
	Sets the alignment of the list that appears upon clicking the
	more tabs button.
	*/
	void setMoreTabsListAlignment(MoreTabsListAlignment moreTabsListAlignment);
	/*
	Returns the names of every tab ordered by tab index.
	*/
	std::vector<std::string> getTabNames();

	bool handleEvent(sf::Event event);

private:
	// The value part of the key:value pairing in the tabsCache cache
	struct CachedTabsValue {
		std::string currentlySelectedTab;
		// Should be ordered the same as tabsOrdering at the time of caching
		// A vector of pairs, a pair for every tab: the tab name and the associated panel
		std::vector<std::pair<std::string, std::shared_ptr<tgui::Panel>>> tabs;
		// The confirmation prompt for every close button
		// If hasCloseButtons is false, this will be an empty vector
		// Otherwise, this contains the string to be displayed for every close button
		// confirmation prompt, with an empty string meaning a prompt shouldn't be displayed
		std::vector<std::string> closeButtonConfirmationPrompts;
	};
	const float EXTRA_HEIGHT_FROM_SCROLLBAR = 16;
	// Spaces appended to the end of every tab name to make room for the close button
	// If font size changes, the number of spaces should be updated and every tab
	// name also should be updated accordingly (even the ones that are cached).
	// This is a lot of work, so it's better to just never change tabs' text size
	std::string tabNameAppendedSpaces;

	EditorWindow& parentWindow;

	// The ScrollablePanel that serves as a container for the tabs object
	std::shared_ptr<tgui::ScrollablePanel> tabsContainer;
	// The actual tabs object
	std::shared_ptr<tgui::Tabs> tabs;
	// Maps tab name to the Panel that will be showed when the tab is selected
	std::map<std::string, std::shared_ptr<tgui::Panel>> panelsMap;
	// Maps tab name to the function to be executed on tab selection
	std::map<std::string, std::function<void()>> onSelectFunctionMap;
	// Tracks the order of the tabs in moreTabsList. Each entry is a tab name at some index.
	std::vector<std::string> tabsOrdering;
	// A vector of pairs of close button and the confirmation prompt message to be shown, one pair for every tab. 
	// The index for closeButtons should match the index for tabsOrdering.
	// An empty confirmation prompt message means no confirmation prompt will be shown if the close button is pressed.
	std::vector<std::pair<std::shared_ptr<tgui::Button>, std::string>> closeButtons;

	// Panel (that belongs to a tab) that is currently active
	std::shared_ptr<tgui::Panel> currentPanel;

	// Button that shows all tabs that can't fit in this widget
	std::shared_ptr<tgui::Button> moreTabsButton;
	// The ListBoxScrollablePanel that is shown when moreTabsButton is clicked
	std::shared_ptr<ListBoxScrollablePanel> moreTabsList;
	// Alignment of the moreTabsList
	MoreTabsListAlignment moreTabsListAlignment = MoreTabsListAlignment::Left;

	// Maps a tab set identifier string to a vector
	// panelsMap, tabsOrdering, the name of the tab that was open (empty string if none)
	Cache<std::string, CachedTabsValue> tabsCache;

	// Signal emitted when a tab is closed via its close button
	// Optional parameter: the name of the tab being closed
	tgui::SignalString onTabClose = { "TabClosed" };

	void onTabSelected(std::string tabName);
	// Should be called whenever there's an update to the tabs
	void onTabsChange();
	/*
	Called when a confirmation prompt for a close button is answered.

	closeButtonConfirmationPromptTargetTabShortenedName - the shortened name (no appended spaces) of the tab being closed
	*/
	void onCloseButtonConfirmationPromptAnswer(bool confirmed, std::string closeButtonConfirmationPromptTargetTabShortenedName);
	/*
	Mark/Unmark a specific tab as requiring pressing "Yes" to a confirmation prompt before
	the tab is actually closed. This function does nothing if hasCloseButtons is false.
	By default, no tab requires a confirmation prompt.

	index - the tab's index in tabs
	message - the string to be displayed in the confirmation prompt.
		Set to an empty string to disable the confirmation prompt.
	*/
	void setTabCloseButtonConfirmationPrompt(int index, std::string message);
	/*
	Create a close button for some tab.

	index - the tab's index in tabs
	*/
	void createCloseButton(int index);
};