#pragma once
#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include "SpriteLoader.h"
#include "Animatable.h"
#include "Animation.h"
#include "AudioPlayer.h"
#include "TimeFunctionVariable.h"
#include "UndoStack.h"
#include "MovementSystem.h"
#include "RenderSystem.h"
#include "CollisionSystem.h"
#include "EnemySystem.h"
#include "DespawnSystem.h"
#include "SpriteAnimationSystem.h"
#include "EntityCreationQueue.h"
#include "ShadowTrailSystem.h"
#include "PlayerSystem.h"
#include "AudioPlayer.h"
#include "CollectibleSystem.h"
#include "DebugRenderSystem.h"
#include "RenderSystem.h"
#include "Attack.h"
#include "EditorMovablePointAction.h"
#include "EditorMovablePoint.h"
#include "MovablePoint.h"
#include "ViewController.h"
#include "LRUCache.h"
#include <memory>
#include <thread>
#include <mutex>
#include <entt/entt.hpp>
#include <tuple>

/*
Sends window to the foreground of the computer display.
*/
void sendToForeground(sf::RenderWindow& window);
/*
Returns a tooltip containing some text.
*/
std::shared_ptr<tgui::Label> createToolTip(std::string text);
/*
Returns a menu popup with clickable buttons.

elements - a vector of pairs containing the text of the button and the function to be called when the button is pressed
*/
std::shared_ptr<tgui::ListBox> createMenuPopup(std::vector<std::pair<std::string, std::function<void()>>> elements);
/*
Returns a sf::VertexArray that contains the positions that an entity following the array of EMPAs will be in over time.
Note that there must be an entity with the PlayerTag component if any of the EMPAs or their EMPAAngleOffsets require the player's position.
The vertices linearly interpolate from startColor to endColor as the path progresses.

timeResolution - the amount of time between each point
x, y - the starting position of the returned vertex array
playerX, playerY - position of the player
*/
sf::VertexArray generateVertexArray(std::vector<std::shared_ptr<EMPAction>> actions, float timeResolution, float x, float y, float playerX, float playerY, sf::Color startColor = sf::Color::Red, sf::Color endColor = sf::Color::Blue);
sf::VertexArray generateVertexArray(std::shared_ptr<EMPAction> action, float timeResolution, float x, float y, float playerX, float playerY, sf::Color startColor = sf::Color::Red, sf::Color endColor = sf::Color::Blue);
/*
Returns an array of segment data. Each segment data is 2 vectors of floats in order:
the x-coordinates and the y-coordinates of a matplotlibc curve.
The x-axis represents time in seconds and the y-axis the TFV's value.

timeResolution - the amount of time between each point
colors - the list of colors that will be looped to determine the color of each segment of the curve
*/
std::vector<std::pair<std::vector<float>, std::vector<float>>> generateMPLPoints(std::shared_ptr<PiecewiseTFV> tfv, float tfvLifespan, float timeResolution);

class EditorWindow;
class UndoableEditorWindow;

/*
A tgui::Slider that allows for values not limited by multiples of the slider's step.
*/
class Slider : public tgui::Slider {
public:
	void setValue(float value);
	using tgui::Slider::setValue;

	void setStep(float step);
	using tgui::Slider::setStep;
};

/*
A tgui::Picture that is able to display animations.

Warning: This widget's update() must be called every render call.
*/
class AnimatablePicture : public tgui::Picture {
public:
	void update(float deltaTime);

	void setAnimation(SpriteLoader& spriteLoader, const std::string& animationName, const std::string& spriteSheetName);
	void setSprite(SpriteLoader& spriteLoader, const std::string& spriteName, const std::string& spriteSheetName);

private:
	std::unique_ptr<Animation> animation;
};

/*
An edit box that accepts only numbers and can have upper/lower limits.

Note: connect() should not be used; use getOnValueSet()'s signal instead.
setText() should not be used either; use setValue() instead.
*/
class NumericalEditBoxWithLimits : public tgui::EditBox {
public:
	NumericalEditBoxWithLimits();

	float getValue();
	void setValue(int value);
	void setValue(float value);
	void setMin(float min);
	void setMax(float max);
	void removeMin();
	void removeMax();
	/*
	Makes the edit box accept only integers.
	This is false by default.
	*/
	void setIntegerMode(bool intMode);

	inline bool getHasMin() { return hasMin; }
	inline bool getHasMax() { return hasMax; }

	std::shared_ptr<entt::SigH<void(float)>> getOnValueSet();

private:
	using tgui::EditBox::connect;
	using tgui::EditBox::setText;

	void updateInputValidator();

	std::shared_ptr<entt::SigH<void(float)>> onValueSet;

	bool hasMin = false, hasMax = false;
	float min, max;
	// if true, the inputted number must be an integer
	bool mustBeInt = false;
};

/*
Widget that 
The AnimatablePicture from getAnimatablePicture() is not added to the AnimatableChooser Group, so it must be added to the AnimatableChooser's
container separately. For the same reason, its size and positions do not change with AnimatableChooser's changes.
*/
class AnimatableChooser : public tgui::Group {
public:
	/*
	forceSprite - if true, the user is forced to choose between sprites instead of being able to choose between sprites and animations
	*/
	AnimatableChooser(SpriteLoader& spriteLoader, bool forceSprite = false, float guiPaddingX = 20, float guiPaddingY = 10);

	void setSelectedItem(Animatable animatable);

	/*
	Should be called whenever this widget's container is resized.
	This function automatically sets the height of this widget.
	*/
	void onContainerResize(int containerWidth, int containerHeight);

	std::shared_ptr<AnimatablePicture> getAnimatablePicture();
	/*
	Returns an Animatable with empty sprite name and sprite sheet name if none is selected.
	*/
	Animatable getValue();
	inline std::shared_ptr<entt::SigH<void(Animatable)>> getOnValueSet() { return onValueSet; }

	void setVisible(bool visible);
	void setEnabled(bool enabled);

private:
	/*
	Calculates and sets the number of items to display, depending on this widget's position relative to its container.
	*/
	void calculateItemsToDisplay();

	SpriteLoader& spriteLoader;
	bool forceSprite;

	std::shared_ptr<tgui::ComboBox> animatable;
	std::shared_ptr<tgui::ComboBox> rotationType;

	std::shared_ptr<AnimatablePicture> animatablePicture;

	float paddingX, paddingY;

	// func takes 1 arg: the new Animatable
	std::shared_ptr<entt::SigH<void(Animatable)>> onValueSet;
};

/*
A slider whose value can be set with a NumericalEditBoxWithLimits located on its right.

The EditBox from getEditBox() is not added to the SliderWithEditBox since SliderWithEditBox is not a container,
so the EditBox must be added to the SliderWithEditBox's container separately. 

Note: connect() should not be used on neither this nor the edit box; use getOnValueSet()'s signal instead.
To get the value, use tgui::Slider::getValue(), not the edit box's value.
Only SliderWithEditBox::setSize() and setPosition() should be called, not any of tgui::Slider's setSize()'s or setPosition()'s
*/
class SliderWithEditBox : public Slider {
public:
	SliderWithEditBox(float paddingX = 20);

	void setSize(float x, float y);
	void setPosition(float x, float y);
	void setValue(float value);
	void setEnabled(bool enabled);
	void setIntegerMode(bool intMode);

	inline std::shared_ptr<tgui::EditBox> getEditBox() { return editBox; }
	inline std::shared_ptr<entt::SigH<void(float)>> getOnValueSet();

	void setVisible(bool visible);

private:
	using tgui::Slider::connect;
	void onEditBoxValueSet(float value);

	// func takes 1 arg: the new value
	std::shared_ptr<entt::SigH<void(float)>> onValueSet;

	float paddingX;

	std::shared_ptr<NumericalEditBoxWithLimits> editBox;
};

/*
Used to edit SoundSettings
*/
class SoundSettingsGroup : public tgui::Group {
public:
	SoundSettingsGroup(std::string pathToSoundsFolder, float paddingX = 20, float paddingY = 10);
	void initSettings(SoundSettings init);
	void populateFileNames(std::string pathToSoundsFolder);

	/*
	Should be called whenever this widget's container is resized.
	This function automatically sets the height of this widget.
	*/
	void onContainerResize(int containerWidth, int containerHeight);

	std::shared_ptr<entt::SigH<void(SoundSettings)>> getOnNewSoundSettingsSignal();

	void setEnabled(bool enabled);

private:
	void onVolumeChange(float volume);
	void onPitchChange(float pitch);

	bool ignoreSignal = false;

	float paddingX, paddingY;

	// func takes 1 arg: the new SoundSettings
	std::shared_ptr<entt::SigH<void(SoundSettings)>> onNewSoundSettings;

	std::shared_ptr<tgui::CheckBox> enableAudio;
	std::shared_ptr<tgui::ComboBox> fileName;
	std::shared_ptr<SliderWithEditBox> volume;
	std::shared_ptr<SliderWithEditBox> pitch;

	std::shared_ptr<tgui::Label> enableAudioLabel;
	std::shared_ptr<tgui::Label> fileNameLabel;
	std::shared_ptr<tgui::Label> volumeLabel;
	std::shared_ptr<tgui::Label> pitchLabel;
};


/*
A tgui::ListBox that can scroll horizontally as well as vertically.
The ListBox from getListBox() does not have to be added to a container, since it is already a part of the super ScrollablePanel.
*/
class ScrollableListBox : public tgui::ScrollablePanel {
public:
	ScrollableListBox();
	static std::shared_ptr<ScrollableListBox> create() {
		return std::make_shared<ScrollableListBox>();
	}

	/*
	Should be called anytime an item is added, removed, or changed from the ListBox.
	*/
	void onListBoxItemsChange();
	/*
	Should be called after the ScrollableListBox is resized.
	*/
	inline void onResize() { onListBoxItemsChange(); }
	bool mouseWheelScrolled(float delta, tgui::Vector2f pos) override;

	void setTextSize(int textSize);
	inline std::shared_ptr<tgui::ListBox> getListBox() { return listBox; }

private:
	std::shared_ptr<tgui::ListBox> listBox;
	std::shared_ptr<tgui::Label> textWidthChecker;
	float longestWidth;
};

/*
Used to edit TFVs.
This widget will use the widgets in tfvEditorWindow to .
It is recommended that if there are multiple TFVGroups being used, that TFVs
*/
class TFVGroup : public tgui::Group {
public:
	TFVGroup(std::shared_ptr<std::recursive_mutex> tguiMutex, float paddingX = 20, float paddingY = 10);

	void beginEditing();
	/*
	saveEditedTFV - if true, the onEditingEnd signal will emit the (unmodified TFV, unmodified TFV). If false, it will emit (unmodified TFV, modified TFV) instead
	*/
	void endEditing(bool saveEditedTFV);
	/*
	Same thing as endEditing(false).
	*/
	void endEditingWithoutSaving();

	/*
	Should be called whenever this widget's container is resized.
	This function automatically sets the height of this widget.
	*/
	void onContainerResize(int containerWidth, int containerHeight);

	/*
	Initialize this widget to tfv's values.
	tfv won't be modified by this widget

	tfvIdentifier - some unique string so that whatever's using TFVGroup knows what TFV is being edited.
		eg We are editing a MoveCustomPolarEMPA's distance TFV using this TFVGroup, so we pass "distance"
		into tfvIdentifier. Then when we catch the onSave signal, we see that the identifier is "distance"
		so we know to set the MoveCustomPolarEMPA's distance TFV to the newly updated TFV.
	*/
	void setTFV(std::shared_ptr<TFV> tfv, float tfvLifespan, std::string tfvIdentifier);
	inline std::shared_ptr<entt::SigH<void()>> getOnEditingStart() { return onEditingStart; }
	inline std::shared_ptr<entt::SigH<void(std::shared_ptr<TFV>, std::shared_ptr<TFV>, std::string, bool)>> getOnEditingEnd() { return onEditingEnd; }
	inline std::shared_ptr<entt::SigH<void(std::shared_ptr<TFV>, std::shared_ptr<TFV>, std::string)>> getOnSave() { return onSave; }

private:
	const float TFV_TIME_RESOLUTION = 0.05f; // Time between each tfv curve vertex

	float paddingX, paddingY;

	std::shared_ptr<std::recursive_mutex> tguiMutex;
	std::recursive_mutex tfvMutex;

	UndoStack undoStack;
	std::thread tfvEditorWindowThread;
	std::shared_ptr<UndoableEditorWindow> tfvEditorWindow;

	std::shared_ptr<tgui::ScrollablePanel> panel;
	std::shared_ptr<tgui::Button> showGraph;
	std::shared_ptr<tgui::Button> finishEditing;
	std::shared_ptr<tgui::Button> saveTFV;

	std::shared_ptr<tgui::Button> addSegment;
	std::shared_ptr<tgui::Button> deleteSegment;
	std::shared_ptr<tgui::Label> startTimeLabel;
	std::shared_ptr<SliderWithEditBox> startTime;
	std::shared_ptr<tgui::Button> changeSegmentType;
	std::shared_ptr<ScrollableListBox> segmentTypePopup;

	std::shared_ptr<ScrollableListBox> segmentList; // Each item ID is the index of the segment in tfv's segment vector
	std::shared_ptr<tgui::Label> tfvFloat1Label;
	std::shared_ptr<SliderWithEditBox> tfvFloat1Slider;
	std::shared_ptr<tgui::Label> tfvFloat2Label;
	std::shared_ptr<SliderWithEditBox> tfvFloat2Slider;
	std::shared_ptr<tgui::Label> tfvFloat3Label;
	std::shared_ptr<SliderWithEditBox> tfvFloat3Slider;
	std::shared_ptr<tgui::Label> tfvFloat4Label;
	std::shared_ptr<SliderWithEditBox> tfvFloat4Slider;
	std::shared_ptr<tgui::Label> tfvInt1Label;
	std::shared_ptr<SliderWithEditBox> tfvInt1Slider;

	std::shared_ptr<TFV> oldTFV; // Should never be modified after setTFV() is called
	std::shared_ptr<PiecewiseTFV> tfv;
	std::string tfvIdentifier;
	std::shared_ptr<TFV> selectedSegment;
	int selectedSegmentIndex = -1;
	float tfvLifespan; // Shouldn't be modified except by setTFV()

	std::shared_ptr<tgui::Label> tfvShortDescription;
	std::shared_ptr<tgui::Button> beginEditingButton;

	// Signal emitted after the user begins using this TFVGroup to edit a TFV
	std::shared_ptr<entt::SigH<void()>> onEditingStart;
	// Signal emitted after the user stops using this TFVGroup to edit a TFV
	// 3 parameters in order: the old TFV with no changes applied, the old TFV with changes applied, and whether to apply changes in whatever's using TFVGroup
	std::shared_ptr<entt::SigH<void(std::shared_ptr<TFV>, std::shared_ptr<TFV>, std::string, bool)>> onEditingEnd;
	// Signal emitted after the user saves the TFV
	std::shared_ptr<entt::SigH<void(std::shared_ptr<TFV>, std::shared_ptr<TFV>, std::string)>> onSave;

	bool ignoreSignal = false;

	void deselectSegment();
	void selectSegment(int index);
	void populateSegmentList();

	void onTFVEditorWindowResize(int windowWidth, int windowHeight);
	void onTFVFloat1SliderChange(float value);
	void onTFVFloat2SliderChange(float value);
	void onTFVFloat3SliderChange(float value);
	void onTFVFloat4SliderChange(float value);
	void onTFVInt1SliderChange(float value);
	void onSelectedSegmentStartTimeChange(float value);
};

/*
Used to edit EMPAAngleOffsets.
*/
class EMPAAngleOffsetGroup : public tgui::Group {
public:
	EMPAAngleOffsetGroup();

	/*
	Should be called whenever this widget's container is resized.
	This function automatically sets the height of this widget.
	*/
	void onContainerResize(int containerWidth, int containerHeight);

	/*
	Initialize this widget to offset's values.
	offset won't be modified by this widget
	*/
	void setEMPAAngleOffset(std::shared_ptr<EMPAAngleOffset> offset);
	inline std::shared_ptr<entt::SigH<void(std::shared_ptr<EMPAAngleOffset>, std::shared_ptr<EMPAAngleOffset>)>> getOnAngleOffsetChange() { return onAngleOffsetChange; }

private:
	std::shared_ptr<EMPAAngleOffset> oldAngleOffset; // Should never be modified after setEMPAAngleOffset() is called
	std::shared_ptr<EMPAAngleOffset> angleOffset;

	std::shared_ptr<tgui::ComboBox> offsetType;
	std::shared_ptr<SliderWithEditBox> x;
	std::shared_ptr<SliderWithEditBox> y;

	// Signal emitted AFTER a change is made to the EMPAAngleOffset
	// 2 parameters in order: the old EMPAAngleOffset with no changes applied and the old EMPAAngleOffset with changes applied
	std::shared_ptr<entt::SigH<void(std::shared_ptr<EMPAAngleOffset>, std::shared_ptr<EMPAAngleOffset>)>> onAngleOffsetChange;
};

/*
A panel that can load Levels and play them while rendering it either normally or in debug mode.
*/
class SimpleEngineRenderer : public tgui::Panel {
public:
	SimpleEngineRenderer(sf::RenderWindow& parentWindow, bool userControlleView = true, bool useDebugRenderSystem = true);
	inline static std::shared_ptr<SimpleEngineRenderer> create(sf::RenderWindow& parentWindow) {
		return std::make_shared<SimpleEngineRenderer>(parentWindow);
	}

	virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

	void handleEvent(sf::Event event);

	void loadLevelPack(std::string name);
	void loadLevelPack(std::shared_ptr<LevelPack> levelPack);

	void loadLevel(int levelIndex);
	void loadLevel(std::shared_ptr<Level> level);

	void pause();
	void unpause();

protected:

private:
	sf::RenderWindow& parentWindow;

	bool paused;

	std::shared_ptr<LevelPack> levelPack;
	std::unique_ptr<SpriteLoader> spriteLoader;
	std::unique_ptr<EntityCreationQueue> queue;

	mutable entt::DefaultRegistry registry;

	sf::FloatRect viewportFloatRect, viewFloatRect;

	bool useDebugRenderSystem;
	std::unique_ptr<MovementSystem> movementSystem;
	std::unique_ptr<RenderSystem> renderSystem;
	std::unique_ptr<CollisionSystem> collisionSystem;
	std::unique_ptr<DespawnSystem> despawnSystem;
	std::unique_ptr<EnemySystem> enemySystem;
	std::unique_ptr<SpriteAnimationSystem> spriteAnimationSystem;
	std::unique_ptr<ShadowTrailSystem> shadowTrailSystem;
	std::unique_ptr<PlayerSystem> playerSystem;
	std::unique_ptr<CollectibleSystem> collectibleSystem;
	std::unique_ptr<AudioPlayer> audioPlayer;

	bool userControlledView;
	std::unique_ptr<ViewController> viewController;
	sf::View viewFromViewController;

	void physicsUpdate(float deltaTime) const;
	void updateWindowView();
};

/*
A widget with tabs that, when one is selected, shows a Panel associated with that tab.
Warning: undefined behavior when there are multiple tabs with the same name, or when 
a tab has an empty string for a name.

This widget is intended to be used for a small number of tabs. It might get
laggy when there's a lot.
*/
class TabsWithPanel : public tgui::Group {
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
	Mark/Unmark a specific tab as requiring pressing "Yes" to a confirmation prompt before
	the tab is actually closed. This function does nothing if hasCloseButtons is false.
	By default, no tab requires a confirmation prompt.

	tabName - the unique name of the tab
	message - the string to be displayed in the confirmation prompt.
		Set to an empty string to disable the confirmation prompt.
	*/
	void setTabCloseButtonConfirmationPrompt(std::string tabName, std::string message);

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

	/*
	Returns the unique name of the currently selected tab.
	*/
	std::string getSelectedTab();
	/*
	Sets the alignment of the list that appears upon clicking the
	more tabs button.
	*/
	void setMoreTabsListAlignment(MoreTabsListAlignment moreTabsListAlignment);

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
	const float TAB_WIDTH = 100.0f;
	// Spaces appended to every tab name to make room for the close button
	// If font size changes, the number of spaces should be updated and every tab
	// name also should be updated accordingly (even the ones that are cached).
	// This is a lot of work, so it's better to just never change tabs' text size
	std::string tabNameAppendedSpaces;

	std::shared_ptr<tgui::Tabs> tabs;
	// Maps tab name to the Panel that will be showed when the tab is selected
	std::map<std::string, std::shared_ptr<tgui::Panel>> panelsMap;
	// Tracks the order of the tabs in moreTabsList
	std::vector<std::string> tabsOrdering;
	// The close buttons, one for every tab. The index for closeButtons should match
	// the index for tabsOrdering
	std::vector<std::pair<std::shared_ptr<tgui::Button>, std::string>> closeButtons;

	// Panel that is currently active
	std::shared_ptr<tgui::Panel> currentPanel;

	// Button that shows all tabs that can't fit in this widget
	std::shared_ptr<tgui::Button> moreTabsButton;
	// The ScrollableListBox that is shown when moreTabsButton is clicked
	std::shared_ptr<ScrollableListBox> moreTabsList;
	// Alignment of the moreTabsList
	MoreTabsListAlignment moreTabsListAlignment = MoreTabsListAlignment::Left;

	// Maps a tab set identifier string to a vector
	// panelsMap, tabsOrdering, the name of the tab that was open (empty string if none)
 	Cache<std::string, CachedTabsValue> tabsCache;

	// Signal emitted when a tab is closed via its close button
	// Optional parameter: the name of the tab being closed
	tgui::SignalString onTabClose = { "TabClosed" };

	void onTabSelected(std::string tabName);
	void updateMoreTabsList();
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

/*
A timeline whose elements are clickable buttons.

Signals:
"ElementClicked" - emitted when an element is clicked; 
	Optional parameter: the index of the element as given in setElements() (int)
*/
class ClickableTimeline : public tgui::Group {
public:
	ClickableTimeline();
	static std::shared_ptr<ClickableTimeline> create() {
		return std::make_shared<ClickableTimeline>();
	}

	/*
	Sets the elements in this timeline. The duration of each element is assumed to be such that it ends right as 
	the next element begins. The only exception is the last element, whose duration ends at maxTime.

	itemStartTimes - the start time and text of every element;
		this vector must be sorted in nondecreasing order by the start time and no element can overlap in time
	maxDuration - the maximum time of the timeline
	*/
	void setElements(std::vector<std::pair<float, std::string>> elementStartTimes, float maxTime);

	/*
	Sets the elements in this timeline. The duration of each element must be given.
	The timeline's max time is assumed to be the last element's start time added to its duration.

	elementStartTimesAndDuration - a vector of tuples representing the start time, duration, and text of each element, respectively;
		this vector must be sorted in nondecreasing order by the start time and no element can overlap in time
	*/
	void setElements(std::vector<std::tuple<float, float, std::string>> elementStartTimesAndDuration);

	/*
	Sets the elements in this timeline. The duration of each element must be given.

	elementStartTimesAndDuration - a vector of tuples representing the start time, duration, and text of each element, respectively;
		this vector must be sorted in nondecreasing order by the start time and no element can overlap in time
	maxTime - the maximum time that can be displayed in the timeline
	*/
	void setElements(std::vector<std::tuple<float, float, std::string>> elementStartTimesAndDuration, float maxTime);

	tgui::Signal& getSignal(std::string signalName) override;

private:
	tgui::SignalInt onElementPressed = { "ElementPressed" };

	// A vector of tuples containing start time, duration, and corresponding Button of each element, in that order
	// This vector is not guaranteed to be sorted in any way
	std::vector<std::tuple<float, float, std::shared_ptr<tgui::Button>>> elements;

	std::shared_ptr<tgui::ScrollablePanel> panel;
	std::shared_ptr<tgui::RangeSlider> cameraController;

	// How much to scale the x-coordinates and width of every button in the panel by
	float buttonScalar = 1.0f;
	// Another scalar that does the same thing; value comes from cameraController
	float buttonScalarScalar = 1.0f;
	// The maximum time value of the timeline
	float maxTimeValue = 1.0f;

	void updateButtonsPositionsAndSizes();
};