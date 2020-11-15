#pragma once
#include <TGUI/TGUI.hpp>

#include <DataStructs/UndoStack.h>
#include <Editor/EventCapturable.h>
#include <Editor/ViewController.h>
#include <Editor/CopyPaste.h>
#include <Editor/Util/ExtraSignals.h>
#include <Editor/CustomWidgets/ListView.h>
#include <Editor/CustomWidgets/NumericalEditBoxWithLimits.h>
#include <Editor/CustomWidgets/SimpleWidgetsContainerPanel.h>
#include <Editor/CustomWidgets/ChildWindow.h>
#include <Editor/CustomWidgets/AnimatablePicture.h>
#include <Editor/Windows/EditorWindowConfirmationPromptChoice.h>

class MainEditorWindow;

/*
A panel that can load a sprite sheet and edit its metafile (a SpriteSheet object).

Signals:
MetafileModified - emitted when the SpriteSheet being edited is modified.
	Optional parameter: a shared_ptr to the newly modified SpriteSheet
*/
class SpriteSheetMetafileEditor : public tgui::Panel, public EventCapturable, public CopyPasteable {
public:
	/*
	Signal emitted when an EditorAttackPattern in the list of attack users is to be edited.
	Optional parameter: the ID of the EditorAttackPattern
	*/
	tgui::SignalSpriteSheet onMetafileModify = { "MetafileModified" };

	SpriteSheetMetafileEditor(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize = 50);
	static std::shared_ptr<SpriteSheetMetafileEditor> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize = 50) {
		return std::make_shared<SpriteSheetMetafileEditor>(mainEditorWindow, clipboard, undoStackSize);
	}
	
	bool updateTime(tgui::Duration elapsedTime) override;
	void draw(tgui::BackendRenderTargetBase& target, tgui::RenderStates states) const override;

	CopyOperationResult copyFrom() override;
	PasteOperationResult pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	PasteOperationResult paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	void loadSpriteSheet(const std::string& levelPackName, std::shared_ptr<SpriteLoader> spriteLoader, std::shared_ptr<SpriteSheet> spriteSheet);
	void loadImage(const std::string& levelPackName, std::shared_ptr<SpriteLoader> spriteLoader, std::string spriteSheetName);

	void repopulateAnimatablesListView();

	void resetCamera();

	void scaleTransparentTextureToCameraZoom(float cameraZoomAmount);

	/*
	Throws std::runtime_error.
	*/
	void renameSprite(std::string oldSpriteName, std::string newSpriteName);

	tgui::Signal& getSignal(tgui::String signalName) override;
	
private:
	// If an item ID in animatablesListView starts with this, the item is a sprite
	const static char ANIMATABLES_LIST_SPRITE_INDICATOR;
	// If an item ID in animatablesListView starts with this, the item is an animation
	const static char ANIMATABLES_LIST_ANIMATION_INDICATOR;
	// Time it takes for selectionRectangleColor/worldSelectionCursorColor to change colors once
	const static float WORLD_SELECTION_COLORS_CHANGE_INTERVAL;
	// Width of selection rectangle borders in pixels
	const static int SELECTION_RECTANGLE_WIDTH;
	// Diameter of the world selection circle cursor (for sprite rect or origin selection)
	const static float WORLD_SELECTION_CURSOR_DIAMETER;
	const static float WORLD_SELECTION_CURSOR_BORDER_THICKNESS;
	// The larger this number, the less viewController's max zoom level scales on the loaded image's size
	const static float MAX_CAMERA_ZOOM_SCALAR;

	MainEditorWindow& mainEditorWindow;
	Clipboard& clipboard;
	UndoStack undoStack;
	// Not guaranteed to contain a loaded texture
	std::shared_ptr<SpriteSheet> spriteSheet;
	std::shared_ptr<SpriteLoader> spriteLoader;

	sf::View viewFromViewController;
	sf::FloatRect viewportFloatRect, viewFloatRect;

	std::unique_ptr<ViewController> viewController;

	std::shared_ptr<ListView> animatablesListView;
	std::shared_ptr<SimpleWidgetsContainerPanel> utilityWidgetsPanel;
	std::shared_ptr<tgui::ColorPicker> backgroundColorPicker;
	std::shared_ptr<tgui::Button> chooseSpriteRectButton;
	std::shared_ptr<tgui::Button> chooseSpriteOriginButton;
	std::shared_ptr<tgui::Button> openSpriteColorPickerButton;
	std::shared_ptr<tgui::Label> spriteOriginLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> spriteOriginXEditBox;
	std::shared_ptr<NumericalEditBoxWithLimits> spriteOriginYEditBox;
	std::shared_ptr<tgui::Label> ingameSpriteSizeLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> ingameSpriteSizeXEditBox;
	std::shared_ptr<NumericalEditBoxWithLimits> ingameSpriteSizeYEditBox;

	std::shared_ptr<ChildWindow> animatablePreviewChildWindow;
	std::shared_ptr<AnimatablePicture> animatablePreviewPicture;

	// Loaded from loadImage()
	sf::Texture* loadedTexture = nullptr;
	// The sprite with loadedTexture
	tgui::Sprite fullTextureAsSprite;

	// Texture used for the background 
	sf::Texture backgroundTexture;
	sf::Sprite backgroundSprite;
	tgui::Color lastChosenBackgroundColor;

	// Texture used for the background of transparent parts of the sprite sheet
	sf::Texture transparentTexture;
	sf::Sprite transparentTextureSprite;

	// Rectangles being drawn
	std::vector<sf::RectangleShape> selectionRectangles;
	tgui::Color selectionRectangleColor = sf::Color::Black;

	std::shared_ptr<SpriteData> selectedSpriteData;
	std::shared_ptr<AnimationData> selectedAnimationData;

	bool selectingSpriteRectTopLeft = false;
	bool selectingSpriteRectBottomRight = false;
	bool selectingSpriteRectOrigin = false;
	// Sprite sheet coordinates of the sprite being edited's area rect's top-left position; only used when editing a sprite's rect
	sf::Vector2i spriteRectTopLeftSet;
	// The cursor used to indicate the world position of the mouse when selecting sprite rect or origin.
	// Is always the opposite color of selectionRectangleColor.
	tgui::Color worldSelectionCursorColor = sf::Color::White;

	// Timer used for updating selectionRectangleColor and worldSelectionCursorColor
	float timeUntilWorldSelectionColorsChange = 0;

	bool ignoreSignals = false;

	void updateWindowView();

	void beginSelectingSpriteRectTopLeft();
	void beginSelectingSpriteRectBottomRight();
	void beginSelectingSpriteRectOrigin();
	// Cancels beginSelectingSpriteRectTopLeft(), beginSelectingSpriteRectBottomRight(), and/or beginSelectingSpriteRectOrigin()
	void stopEditingSpriteProperties();

	void onAnimatableDeselect();
	void onSpriteSelect(std::shared_ptr<SpriteData> spriteData);
	void onAnimationSelect(std::shared_ptr<AnimationData> animationData);
	void onLeftClick(int mouseX, int mouseY);

	void onNewSpriteNameInput(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, std::string spriteName);
	void onNewAnimationNameInput(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, std::string animationName);
	void onNewPastedSpriteNameInput(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, 
		std::string spriteName, std::shared_ptr<SpriteData> pastedSprite);
	void onNewPastedAnimationNameInput(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, 
		std::string animationName, std::shared_ptr<AnimationData> pastedAnimation);

	/*
	Returns the top-left scren position where the world (the sprite sheet view) is
	drawn to.
	*/
	sf::Vector2f getWorldViewOffsetInPixels() const;
	std::string getAnimatablesListViewSpriteItemId(std::string spriteName) const;
	std::string getAnimatablesListViewAnimationItemId(std::string animationName) const;
};