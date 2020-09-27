#pragma once
#include <Editor/CustomWidgets/HideableGroup.h>
#include <Editor/CustomWidgets/AnimatablePicture.h>
#include <Editor/Util/ExtraSignals.h>
#include <LevelPack/Animatable.h>

/*
Widget for choosing an Animatable from the list of all Animatables in a SpriteLoader.

Signals:
	ValueChanged - emitted when a change is made to the Animatable object being edited
	Optional parameter: the new Animatable object

The height of this widget will always be just enough to fit all
the widgets in it, so changing the height of this widget will do nothing.
*/
class AnimatableChooser : public HideableGroup {
public:
	tgui::SignalAnimatable onValueChange = { "ValueChanged" };

	/*
	forceSprite - if true, the user is forced to choose between sprites instead of being able to choose between sprites and animations
	*/
	AnimatableChooser(SpriteLoader& spriteLoader, bool forceSprite = false);
	static std::shared_ptr<AnimatableChooser> create(SpriteLoader& spriteLoader, bool forceSprite = false) {
		return std::make_shared<AnimatableChooser>(spriteLoader, forceSprite);
	}

	void repopulateAnimatables();

	/*
	Sets the value of the Animatable being edited.
	*/
	void setValue(Animatable animatable);
	/*
	Returns an Animatable with empty sprite name and sprite sheet name if none is selected.
	*/
	Animatable getValue();

	void setEnabled(bool enabled);
	void setAnimatablePictureSize(tgui::Layout width, tgui::Layout height);
	void setAnimatablePictureSize(const tgui::Layout2d& size);

	tgui::Signal& getSignal(tgui::String signalName) override;

private:
	/*
	Calculates and sets the number of items to display, depending on this widget's position relative to its container.
	*/
	void calculateItemsToDisplay();

	SpriteLoader& spriteLoader;
	bool forceSprite;

	std::shared_ptr<tgui::ComboBox> animatable;
	std::shared_ptr<tgui::ComboBox> rotationType;
	// The ID of the previously selected animatable item
	std::string previousAnimatableSelection = "";

	std::shared_ptr<AnimatablePicture> animatablePicture;

	bool ignoreSignals = false;
};