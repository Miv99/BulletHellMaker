#pragma once
#include <TGUI/TGUI.hpp>
#include <string>
#include "SpriteLoader.h"
#include "Animatable.h"
#include "Animation.h"
#include "AudioPlayer.h"
#include "TimeFunctionVariable.h"
#include "UndoStack.h"
#include "Attack.h"
#include "EditorMovablePointAction.h"
#include <memory>
#include <entt/entt.hpp>

std::shared_ptr<tgui::Label> createToolTip(std::string text);

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

Note: connect() should not be used on neither this nor the edit box; use getOnValueSet()'s signal instead.
To get the value, use tgui::Slider::getValue(), not the edit box's value.
Remember to also add getEditBox() to whatever container this is in.
Only SliderWithEditBox::setSize() and setPosition() should be called, not any of tgui::Slider's setSize()'s or setPosition()'s
*/
class SliderWithEditBox : public tgui::Slider {
public:
	SliderWithEditBox(float paddingX = 20);

	void setSize(float x, float y);
	void setPosition(float x, float y);
	void setValue(float value);
	void setEnabled(bool enabled);

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
Used to edit TFVs.
Note that unlike the other widgets in EditorUtilities, TFVGroup automatically takes care of adding
commands to the undo stack.
This implementation of a TFV editor is pretty specific to TFVs in EditorAttacks' EMPs' EMPAs' TFVs (mainly
because of the callback signal).
*/
class TFVGroup : public tgui::Group {
public:
	TFVGroup(UndoStack& undoStack);

	/*
	Should be called whenever this widget's container is resized.
	This function automatically sets the height of this widget.
	*/
	void onContainerResize(int containerWidth, int containerHeight);

	void setTFV(std::shared_ptr<TFV> tfv, std::shared_ptr<EMPAction> parentEMPA, std::shared_ptr<EditorMovablePoint> parentEMP, std::shared_ptr<EditorAttack> parentAttack);
	inline std::shared_ptr<entt::SigH<void(std::shared_ptr<EMPAction>, std::shared_ptr<EditorMovablePoint>, std::shared_ptr<EditorAttack>)>> getOnTFVChange() { return onTFVChange; }

private:
	UndoStack& undoStack;

	std::shared_ptr<TFV> tfv;
	std::shared_ptr<EMPAction> parentEMPA;
	std::shared_ptr<EditorMovablePoint> parentEMP;
	std::shared_ptr<EditorAttack> parentAttack;

	std::shared_ptr<tgui::Slider> test;

	// Signal emitted AFTER a change is made to the TFV
	// The EMPA parameter is the EMPA that the TFV being edited belongs to. The EMP parameter is the EMP that the EMPA belongs to.
	// The EditorAttack parameter is the EditorAttack that the EMP belongs to.
	std::shared_ptr<entt::SigH<void(std::shared_ptr<EMPAction>, std::shared_ptr<EditorMovablePoint>, std::shared_ptr<EditorAttack>)>> onTFVChange;

	// This bool is used to prevent infinite loops (ie a change from an undo creating an undo command)
	bool ignoreSignal = false;
};

/*
Used to edit EMPAAngleOffsets.
Note that unlike the other widgets in EditorUtilities, TFVGroup automatically takes care of adding
commands to the undo stack.
This implementation of a TFV editor is pretty specific to TFVs in EditorAttacks' EMPs' EMPAs' TFVs (mainly
because of the callback signal).
*/
class EMPAAngleOffsetGroup : public tgui::Group {
public:
	EMPAAngleOffsetGroup(UndoStack& undoStack);

	/*
	Should be called whenever this widget's container is resized.
	This function automatically sets the height of this widget.
	*/
	void onContainerResize(int containerWidth, int containerHeight);

	inline std::shared_ptr<entt::SigH<void(std::shared_ptr<EMPAction>, std::shared_ptr<EditorMovablePoint>, std::shared_ptr<EditorAttack>)>> getOnAngleOffsetChange() { return onAngleOffsetChange; }

private:
	UndoStack& undoStack;

	std::shared_ptr<EMPAAngleOffset> angleOffset;
	std::shared_ptr<EMPAction> parentEMPA;
	std::shared_ptr<EditorMovablePoint> parentEMP;
	std::shared_ptr<EditorAttack> parentAttack;

	// Signal emitted AFTER a change is made to the EMPAAngleOffset
	// The EMPA parameter is the EMPA that the TFV being edited belongs to. The EMP parameter is the EMP that the EMPA belongs to.
	// The EditorAttack parameter is the EditorAttack that the EMP belongs to.
	std::shared_ptr<entt::SigH<void(std::shared_ptr<EMPAction>, std::shared_ptr<EditorMovablePoint>, std::shared_ptr<EditorAttack>)>> onAngleOffsetChange;

};

/*
A tgui::ListBox that can scroll horizontally as well as vertically.
*/
class ScrollableListBox : public tgui::ScrollablePanel {
public:
	//TODO: create a ListBox and insert it into this; every time an item is added/removed, track the width of the item by setting a tgui::Label
	// to have that text, then checking the label's width. this scrollablepanel's width is then set to that max width
	// The ListBox will be the same size as this scrollablepanel
};