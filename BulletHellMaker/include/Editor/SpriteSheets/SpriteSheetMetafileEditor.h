#pragma once
#include <TGUI/TGUI.hpp>

#include <Editor/EventCapturable.h>
#include <Editor/ViewController.h>
#include <Editor/CopyPaste.h>
#include <Editor/Util/ExtraSignals.h>

class MainEditorWindow;

/*
A panel that can load a sprite sheet and edit its metafile (a SpriteSheet object).

Signals:
MetafileModified - emitted when the SpriteSheet being edited is modified.
	Optional parameter: a shared_ptr to the newly modified SpriteSheet
*/
class SpriteSheetMetafileEditor : public tgui::Panel, public EventCapturable, public CopyPasteable {
public:
	SpriteSheetMetafileEditor(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::shared_ptr<SpriteSheet> spriteSheet);
	static std::shared_ptr<SpriteSheetMetafileEditor> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, 
		std::shared_ptr<SpriteSheet> spriteSheet) {

		return std::make_shared<SpriteSheetMetafileEditor>(mainEditorWindow, clipboard, spriteSheet);
	}
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	CopyOperationResult copyFrom() override;
	PasteOperationResult pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	PasteOperationResult paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	void loadImage(std::string fileName);

	void resetCamera();

	tgui::Signal& getSignal(std::string signalName) override;
	
private:
	MainEditorWindow& mainEditorWindow;
	Clipboard& clipboard;
	std::shared_ptr<SpriteSheet> spriteSheet;

	sf::View viewFromViewController;
	sf::FloatRect viewportFloatRect, viewFloatRect;

	std::unique_ptr<ViewController> viewController;

	/*
	Signal emitted when an EditorAttackPattern in the list of attack users is to be edited.
	Optional parameter: the ID of the EditorAttackPattern
	*/
	tgui::SignalSpriteSheet onMetafileModify = { "MetafileModified" };

	// Loaded from loadImage()
	sf::Texture loadedTexture;
	// The sprite with loadedTexture
	sf::Sprite imageAsSprite;
};