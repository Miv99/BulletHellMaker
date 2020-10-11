#pragma once
#include <TGUI/TGUI.hpp>

#include <DataStructs/UndoStack.h>
#include <Editor/EventCapturable.h>
#include <Editor/ViewController.h>
#include <Editor/CopyPaste.h>
#include <Editor/Util/ExtraSignals.h>
#include <Editor/CustomWidgets/ListView.h>
#include <Editor/CustomWidgets/SimpleWidgetsContainerPanel.h>

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

	void loadSpriteSheet(std::shared_ptr<SpriteLoader> spriteLoader, std::shared_ptr<SpriteSheet> spriteSheet);
	void loadImage(std::shared_ptr<SpriteLoader> spriteLoader, std::string spriteSheetName);

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
	// Time it takes for selectionRectangleColor to change color once
	const static float SELECTION_RECTANGLE_COLOR_CHANGE_INTERVAL;
	// Width of selection rectangle borders in pixels
	const static int SELECTION_RECTANGLE_WIDTH;

	MainEditorWindow& mainEditorWindow;
	Clipboard& clipboard;
	UndoStack undoStack;
	// Not guaranteed to contain a loaded texture
	std::shared_ptr<SpriteSheet> spriteSheet;

	sf::View viewFromViewController;
	sf::FloatRect viewportFloatRect, viewFloatRect;

	std::unique_ptr<ViewController> viewController;

	std::shared_ptr<ListView> animatablesListView;
	std::shared_ptr<SimpleWidgetsContainerPanel> utilityWidgetsPanel;
	std::shared_ptr<tgui::ColorPicker> backgroundColorPicker;

	// Loaded from loadImage()
	sf::Texture* loadedTexture;
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
	float timeUntilSelectionRectangleColorChange = 0;

	void updateWindowView();

	void deselectRectangles();
	void selectSpriteRectangles(std::shared_ptr<SpriteData> spriteData);
	void selectAnimationRectangles(std::shared_ptr<AnimationData> animationData);
};