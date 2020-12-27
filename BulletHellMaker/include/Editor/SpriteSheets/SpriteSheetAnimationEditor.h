#pragma once
#include <TGUI/TGUI.hpp>
#include <DataStructs/UndoStack.h>
#include <DataStructs/SpriteLoader.h>
#include <Editor/EventCapturable.h>
#include <Editor/CustomWidgets/AnimatablePicture.h>
#include <Editor/CustomWidgets/NumericalEditBoxWithLimits.h>

class MainEditorWindow;

/*
A panel that can edit a sprite sheet's AnimationData directly (meaning no signals; 
the AnimationData passed into setAnimationData will be directly modified).

Signals:
	AnimationModified - emitted when the AnimationData being edited is modified.
*/
class SpriteSheetAnimationEditor : public tgui::Panel, public EventCapturable {
public:
	tgui::Signal onAnimationModify = { "AnimationModified" };
	
	/*
	spriteLoader - the SpriteLoader having at least animationData loaded. This SpriteLoader will be modified.
	animationData - the AnimationData being edited
	*/
	SpriteSheetAnimationEditor(MainEditorWindow& mainEditorWindow, std::shared_ptr<SpriteLoader> spriteLoader,
		std::shared_ptr<AnimationData> animationData, int undoStackSize = 50);
	static std::shared_ptr<SpriteSheetAnimationEditor> create(MainEditorWindow& mainEditorWindow, std::shared_ptr<SpriteLoader> spriteLoader,
		std::shared_ptr<AnimationData> animationData, int undoStackSize = 50) {

		return std::make_shared<SpriteSheetAnimationEditor>(mainEditorWindow, spriteLoader, animationData, undoStackSize);
	}

	bool handleEvent(sf::Event event) override;

	void setSpriteLoader(std::shared_ptr<SpriteLoader> spriteLoader, const std::string& spriteSheetName);

private:
	struct SpriteRow {
		// The group containing all the other widgets
		std::shared_ptr<tgui::Group> widgetGroup;

		std::shared_ptr<tgui::Label> rowNumberLabel;
		std::shared_ptr<tgui::EditBox> spriteNameEditBox;
		std::shared_ptr<NumericalEditBoxWithLimits> durationEditBox;
		std::shared_ptr<tgui::Button> deleteRowButton;
		std::shared_ptr<tgui::Button> addRowButton;

		int row;
	};

	const static float MAX_ANIMATION_PREVIEW_HEIGHT;
	const static int MAX_SPRITE_ROWS_PER_PAGE;

	int spriteRowsPerPage;

	MainEditorWindow& mainEditorWindow;
	UndoStack undoStack;
	std::shared_ptr<SpriteLoader> spriteLoader;
	std::string spriteSheetName;
	std::shared_ptr<AnimationData> animationData;

	std::shared_ptr<AnimatablePicture> animationPreviewPicture;
	std::vector<SpriteRow> spriteRows;
	// Buttonf for inserting a sprite info at the end of the animation data's list
	std::shared_ptr<tgui::Button> insertSpriteInfoButton;

	std::shared_ptr<tgui::Button> previousPageButton;
	std::shared_ptr<tgui::Label> currentPageLabel;
	std::shared_ptr<tgui::Button> nextPageButton;
	std::shared_ptr<tgui::Label> gotoPageLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> gotoPageEditBox;

	int currentPage = 0; // Starts at 0

	bool ignoreSignals = false;

	void setCurrentPage(int currentPage);
	/*
	Sets the current page to the one that contains row.

	row - the row number, starting from 0
	*/
	void viewRow(int row);
	void insertSpriteInfo(int index, std::pair<float, std::string> newInfo);

	/*
	Should be called any time animationData is modified by this widget.
	*/
	void onAnimationModified();
};