#pragma once
#include <TGUI/TGUI.hpp>
#include <string>
#include "SpriteLoader.h"
#include "Animatable.h"
#include "Animation.h"
#include "AudioPlayer.h"
#include "TimeFunctionVariable.h"
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

class AnimatableChooser : public tgui::ComboBox {
public:
	/*
	forceSprite - if true, the user is forced to choose between sprites instead of being able to choose between sprites and animations
	*/
	AnimatableChooser(SpriteLoader& spriteLoader, bool forceSprite = false);

	/*
	Calculates and sets the number of items to display, depending on this widget's position relative to its container.
	*/
	void calculateItemsToDisplay();
	void setSelectedItem(Animatable animatable);

	std::shared_ptr<AnimatablePicture> getAnimatablePicture();

	void setVisible(bool visible);

private:
	SpriteLoader& spriteLoader;
	bool forceSprite;

	std::shared_ptr<AnimatablePicture> animatablePicture;
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
	SoundSettingsGroup(float paddingX = 20, float paddingY = 10);
	void initSettings(SoundSettings init);

	/*
	Should be called whenever this widget's container is resized.
	This function automatically sets the height of this widget.
	*/
	void onContainerResize(int containerWidth, int containerHeight);

	std::shared_ptr<entt::SigH<void(SoundSettings)>> getOnNewSoundSettingsSignal();

private:
	void onVolumeChange(float volume);
	void onPitchChange(float pitch);

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
*/
class TFVGroup : public tgui::Group {
public:
	TFVGroup();

	/*
	Should be called whenever this widget's container is resized.
	This function automatically sets the height of this widget.
	*/
	void onContainerResize(int containerWidth, int containerHeight);

	inline std::shared_ptr<TFV> getTFV() { return tfv; }
	inline std::shared_ptr<entt::SigH<void()>> getOnTFVChange() { return onTFVChange; }

private:
	std::shared_ptr<TFV> tfv;
	// Signal emitted whenever a change is made to the TFV obtained from getTFV()
	std::shared_ptr<entt::SigH<void()>> onTFVChange;
};

/*
Used to edit EMPAAngleOffsets;
*/
class EMPAAngleOffsetGroup : public tgui::Group {
public:
	EMPAAngleOffsetGroup();

	/*
	Should be called whenever this widget's container is resized.
	This function automatically sets the height of this widget.
	*/
	void onContainerResize(int containerWidth, int containerHeight);

	inline std::shared_ptr<EMPAAngleOffset> getAngleOffset() { return angleOffset; }
	inline std::shared_ptr<entt::SigH<void()>> getOnAngleOffsetChange() { return onAngleOffsetChange; }

private:
	std::shared_ptr<EMPAAngleOffset> angleOffset;
	// Signal emitted whenever a change is made to the EMPAAngleOffset obtained from getAngleOffset()
	std::shared_ptr<entt::SigH<void()>> onAngleOffsetChange;
};