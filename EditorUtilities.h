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
#include "DebugRenderSystem.h"
#include "RenderSystem.h"
#include "Attack.h"
#include "EditorMovablePointAction.h"
#include "EditorMovablePoint.h"
#include "MovablePoint.h"
#include <memory>
#include <thread>
#include <mutex>
#include <entt/entt.hpp>

/*
Sends window to the foreground of the computer display.
*/
void sendToForeground(sf::RenderWindow& window);
/*
Returns a tooltip containing some text.
*/
std::shared_ptr<tgui::Label> createToolTip(std::string text);
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