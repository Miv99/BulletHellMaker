#pragma once
#include <Mutex.h>

#include <Editor/CustomWidgets/TabsWithPanel.h>
#include <Editor/SpriteSheets/SpriteSheetMetafileEditor.h>
#include <Editor/SpriteSheets/SpriteSheetAnimationEditor.h>
#include <Editor/CopyPaste.h>

class MainEditorWindow;

/*
The TabsWithPanel container for editing everything about a sprite sheet's metafile.

Signals:
MetafileModified - emitted when the SpriteSheet being edited is modified.
	Optional parameter: a shared_ptr to the newly modified SpriteSheet
*/
class SpriteSheetMetafileEditorContainer : public tgui::Panel, public EventCapturable {
public:
	tgui::SignalSpriteSheet onMetafileModify = { "MetafileModified" };

	/*
	Throws std::runtime_error.

	spriteLoader - the SpriteLoader having at least spriteSheet loaded. This SpriteLoader will not be modified.
	spriteSheet - the SpriteSheet being edited
	*/
	SpriteSheetMetafileEditorContainer(MainEditorWindow& mainEditorWindow, Clipboard& clipboard,
		const std::string& levelPackName, std::shared_ptr<SpriteLoader> spriteLoader, std::shared_ptr<SpriteSheet> spriteSheet) throw();
	static std::shared_ptr<SpriteSheetMetafileEditorContainer> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard,
		const std::string& levelPackName, std::shared_ptr<SpriteLoader> spriteLoader, std::shared_ptr<SpriteSheet> spriteSheet) {
		
		return std::make_shared<SpriteSheetMetafileEditorContainer>(mainEditorWindow, clipboard, levelPackName, spriteLoader, spriteSheet);
	}

	bool handleEvent(sf::Event event) override;

	/*
	Throws std::runtime_error.
	*/
	void loadImage(const std::string& levelPackName, std::shared_ptr<SpriteLoader> spriteLoader, std::string spriteSheetName) throw();

	/*
	Throws std::runtime_error.
	*/
	void renameSprite(std::string oldSpriteName, std::string newSpriteName) noexcept(false);
	/*
	Throws std::runtime_error.
	*/
	void renameAnimation(std::string oldAnimationName, std::string newAnimationName) noexcept(false);

private:
	const static std::string MAIN_EDITOR_TAB_NAME;
	const static std::string ANIMATION_EDITOR_TAB_PREFIX;

	MainEditorWindow& mainEditorWindow;
	std::shared_ptr<SpriteSheet> spriteSheet;

	std::shared_ptr<TabsWithPanel> tabs;

	std::shared_ptr<SpriteSheetMetafileEditor> metafileEditor;
};