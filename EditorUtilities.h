#pragma once
#include <TGUI/TGUI.hpp>
#include <string>
#include "SpriteLoader.h"
#include "Animatable.h"
#include "Animation.h"
#include <memory>

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

	std::shared_ptr<AnimatablePicture> getAnimatablePicture();

private:
	SpriteLoader& spriteLoader;
	bool forceSprite;

	std::shared_ptr<AnimatablePicture> animatablePicture;
};

class SliderWithEditBox : public tgui::Slider {
public:
	SliderWithEditBox();

	inline std::shared_ptr<tgui::EditBox> getEditBox() { return editBox; }

private:
	std::shared_ptr<tgui::EditBox> editBox;
};