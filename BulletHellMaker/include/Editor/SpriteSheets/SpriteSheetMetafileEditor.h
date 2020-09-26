#pragma once
#include <TGUI/TGUI.hpp>

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

	SpriteSheetMetafileEditor(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::shared_ptr<SpriteSheet> spriteSheet);
	static std::shared_ptr<SpriteSheetMetafileEditor> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, 
		std::shared_ptr<SpriteSheet> spriteSheet) {

		return std::make_shared<SpriteSheetMetafileEditor>(mainEditorWindow, clipboard, spriteSheet);
	}
	
	bool updateTime(tgui::Duration elapsedTime) override;
	void draw(tgui::BackendRenderTargetBase& target, tgui::RenderStates states) const override;

	CopyOperationResult copyFrom() override;
	PasteOperationResult pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	PasteOperationResult paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	void loadImage(std::shared_ptr<SpriteLoader> spriteLoader, std::string spriteSheetName);

	void resetCamera();

	tgui::Signal& getSignal(tgui::String signalName) override;
	
private:
	MainEditorWindow& mainEditorWindow;
	Clipboard& clipboard;
	// Not guaranteed to contain a loaded texture
	std::shared_ptr<SpriteSheet> spriteSheet;

	sf::View viewFromViewController;
	sf::FloatRect viewportFloatRect, viewFloatRect;

	std::unique_ptr<ViewController> viewController;

	std::shared_ptr<ListView> animatablesListView;
	std::shared_ptr<SimpleWidgetsContainerPanel> utilityWidgetsPanel;

	// Loaded from loadImage()
	sf::Texture* loadedTexture;
	// The sprite with loadedTexture
	tgui::Sprite fullTextureAsSprite;

	// Texture used for the background 
	sf::Texture backgroundTexture;
	tgui::Sprite backgroundSprite;

	void updateWindowView();
};